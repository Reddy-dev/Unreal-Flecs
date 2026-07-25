#!/usr/bin/env python3
"""Validate the Unreal-Flecs layout and an upstream Flecs merge."""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path


SPECIAL_MAPPINGS = {
    "src/addons/query_dsl/parser.c": "Source/FlecsLibrary/Private/addons/query_dsl/query_dsl_parser.c",
    "src/addons/script/expr/ast.c": "Source/FlecsLibrary/Private/addons/script/expr/ast_expr.c",
    "src/addons/script/expr/ast.h": "Source/FlecsLibrary/Private/addons/script/expr/ast_expr.h",
    "src/addons/script/expr/expr.h": "Source/FlecsLibrary/Private/addons/script/expr/expr_expr.h",
    "src/addons/script/expr/parser.c": "Source/FlecsLibrary/Private/addons/script/expr/parser_expr.c",
    "src/addons/script/expr/stack.c": "Source/FlecsLibrary/Private/addons/script/expr/stack_expr.c",
    "src/addons/script/expr/stack.h": "Source/FlecsLibrary/Private/addons/script/expr/stack_expr.h",
    "src/addons/script/expr/util.c": "Source/FlecsLibrary/Private/addons/script/expr/util_expr.c",
    "src/addons/script/expr/visit.h": "Source/FlecsLibrary/Private/addons/script/expr/visit_expr.h",
    "src/addons/script/expr/visit_eval.c": "Source/FlecsLibrary/Private/addons/script/expr/visit_eval_expr.c",
    "src/addons/script/expr/visit_fold.c": "Source/FlecsLibrary/Private/addons/script/expr/visit_fold_expr.c",
    "src/addons/script/expr/visit_free.c": "Source/FlecsLibrary/Private/addons/script/expr/visit_free_expr.c",
    "src/addons/script/expr/visit_to_str.c": "Source/FlecsLibrary/Private/addons/script/expr/visit_to_str_expr.c",
    "src/addons/script/expr/visit_type.c": "Source/FlecsLibrary/Private/addons/script/expr/visit_type_expr.c",
}
KNOWN_UNMAPPED = {"include/flecs/addons/os_api_impl.h"}


class Validator:
    def __init__(
        self,
        repository: Path,
        target_ref: str,
        max_root_header_bytes: int,
        allow_missing_test_conversions: bool,
    ) -> None:
        self.repository = repository.resolve()
        self.target_ref = target_ref
        self.max_root_header_bytes = max_root_header_bytes
        self.allow_missing_test_conversions = allow_missing_test_conversions
        self.errors: list[str] = []
        self.warnings: list[str] = []

    def git(
        self, *arguments: str, check: bool = True
    ) -> subprocess.CompletedProcess[str]:
        result = subprocess.run(
            [
                "git",
                "-c",
                f"safe.directory={self.repository.as_posix()}",
                *arguments,
            ],
            cwd=self.repository,
            check=False,
            capture_output=True,
            text=True,
        )
        if check and result.returncode:
            detail = result.stderr.strip() or result.stdout.strip()
            raise RuntimeError(f"git {' '.join(arguments)} failed: {detail}")
        return result

    def git_lines(self, *arguments: str) -> list[str]:
        return [
            line
            for line in self.git(*arguments).stdout.splitlines()
            if line
        ]

    def is_ancestor(self, ancestor: str, descendant: str) -> bool:
        return (
            self.git(
                "merge-base",
                "--is-ancestor",
                ancestor,
                descendant,
                check=False,
            ).returncode
            == 0
        )

    def find_previous_upstream(self) -> str | None:
        merge_head = self.git(
            "rev-parse", "--verify", "--quiet", "MERGE_HEAD", check=False
        )
        if merge_head.returncode == 0:
            return self.git_lines("merge-base", "HEAD", self.target_ref)[0]

        target_commit = self.git_lines("rev-parse", self.target_ref)[0]
        for merge_commit in self.git_lines(
            "rev-list", "--merges", "--first-parent", "HEAD"
        ):
            parents = self.git_lines("show", "-s", "--format=%P", merge_commit)[
                0
            ].split()
            if len(parents) < 2:
                continue
            if any(
                self.is_ancestor(target_commit, merged_parent)
                for merged_parent in parents[1:]
            ):
                return self.git_lines(
                    "merge-base", parents[0], self.target_ref
                )[0]

        merge_base = self.git_lines("merge-base", "HEAD", self.target_ref)[0]
        if merge_base != target_commit:
            return merge_base
        return None

    def validate_conflicts_and_paths(self) -> None:
        unmerged = self.git_lines(
            "diff", "--name-only", "--diff-filter=U"
        )
        if unmerged:
            self.errors.append(
                f"Unresolved conflicts remain: {', '.join(unmerged)}"
            )

        forbidden = self.git_lines(
            "ls-files", "--", "include", "src", "distr", "test"
        )
        if forbidden:
            self.errors.append(
                "Upstream root paths are tracked in the plugin: "
                + ", ".join(forbidden)
            )

    def validate_root_header(self) -> None:
        relative_path = Path("Source/FlecsLibrary/Public/flecs.h")
        header_path = self.repository / relative_path
        if not header_path.is_file():
            self.errors.append(f"{relative_path.as_posix()} is missing.")
            return

        header = header_path.read_text(encoding="utf-8")
        include_count = len(
            re.findall(
                r'^\s*#include\s+"flecs/private/api_defines\.h"\s*$',
                header,
                re.MULTILINE,
            )
        )
        if include_count != 1:
            self.errors.append(
                f"{relative_path.as_posix()} must include "
                "flecs/private/api_defines.h exactly once; "
                f"found {include_count}."
            )
        if re.search(
            r"^\s*#ifndef\s+FLECS_API_DEFINES_H\s*$",
            header,
            re.MULTILINE,
        ):
            self.errors.append(
                f"{relative_path.as_posix()} contains generated "
                "api_defines.h content."
            )
        if header_path.stat().st_size > self.max_root_header_bytes:
            self.errors.append(
                f"{relative_path.as_posix()} is {header_path.stat().st_size} "
                f"bytes; the modular-header limit is "
                f"{self.max_root_header_bytes}."
            )

    @staticmethod
    def map_upstream_path(upstream_path: str) -> str | None:
        if upstream_path in SPECIAL_MAPPINGS:
            return SPECIAL_MAPPINGS[upstream_path]
        if upstream_path.startswith("include/"):
            return "Source/FlecsLibrary/Public/" + upstream_path[8:]
        if upstream_path.startswith("src/"):
            return "Source/FlecsLibrary/Private/" + upstream_path[4:]
        return None

    def validate_mappings(self) -> None:
        missing: list[str] = []
        for upstream_path in self.git_lines(
            "ls-tree",
            "-r",
            "--name-only",
            self.target_ref,
            "--",
            "include",
            "src",
        ):
            if upstream_path in KNOWN_UNMAPPED:
                continue
            local_path = self.map_upstream_path(upstream_path)
            if local_path and not (self.repository / local_path).is_file():
                missing.append(f"{upstream_path} -> {local_path}")
        if missing:
            self.errors.append(
                "Missing mapped upstream files:\n  " + "\n  ".join(missing)
            )

    def manifest_at(self, reference: str) -> dict:
        result = self.git("show", f"{reference}:test/cpp/project.json")
        return json.loads(result.stdout)

    @staticmethod
    def manifest_cases(manifest: dict) -> set[str]:
        return {
            f"{suite['id']}::{case}"
            for suite in manifest["test"]["testsuites"]
            for case in suite["testcases"]
        }

    def validate_new_tests(self) -> None:
        previous_upstream = self.find_previous_upstream()
        if previous_upstream is None:
            self.warnings.append(
                "Could not identify the upstream baseline that preceded "
                f"{self.target_ref}; new-test comparison was skipped."
            )
            return

        previous_cases = self.manifest_cases(
            self.manifest_at(previous_upstream)
        )
        target_cases = self.manifest_cases(
            self.manifest_at(self.target_ref)
        )
        spec_directory = self.repository / "Source/FlecsLibrary/Tests/Specs"
        spec_source = "\n".join(
            path.read_text(encoding="utf-8")
            for path in spec_directory.glob("*.spec.cpp")
        )

        missing: list[str] = []
        for qualified_case in sorted(target_cases - previous_cases):
            suite, case = qualified_case.split("::", 1)
            function_name = re.escape(f"{suite}_{case}")
            if not re.search(
                rf"^\s*void\s+{function_name}\s*\(",
                spec_source,
                re.MULTILINE,
            ):
                missing.append(qualified_case)

        if missing:
            message = (
                "New upstream C++ tests lack CQTest conversions: "
                + ", ".join(missing)
            )
            if self.allow_missing_test_conversions:
                self.warnings.append(message)
            else:
                self.errors.append(message)

    def validate_whitespace(self) -> None:
        result = self.git("diff", "--cached", "--check", check=False)
        if result.returncode:
            self.errors.append(
                "git diff --cached --check found whitespace errors."
            )

    def run(self) -> int:
        detected_root = Path(
            self.git_lines("rev-parse", "--show-toplevel")[0]
        ).resolve()
        if detected_root != self.repository:
            self.errors.append(
                f"Expected repository root '{self.repository}', but Git "
                f"reported '{detected_root}'."
            )
        else:
            self.validate_conflicts_and_paths()
            self.validate_root_header()
            self.validate_mappings()
            self.validate_new_tests()
            self.validate_whitespace()

        for warning in self.warnings:
            print(f"WARNING: {warning}", file=sys.stderr)
        for error in self.errors:
            print(f"ERROR: {error}", file=sys.stderr)
        if self.errors:
            print(
                f"Upstream merge validation failed with "
                f"{len(self.errors)} error(s).",
                file=sys.stderr,
            )
            return 1

        print("Upstream merge validation passed.")
        return 0


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--repository",
        type=Path,
        default=Path(__file__).resolve().parent.parent,
    )
    parser.add_argument("--target-ref", default="upstream/master")
    parser.add_argument("--max-root-header-bytes", type=int, default=307200)
    parser.add_argument(
        "--allow-missing-test-conversions",
        action="store_true",
    )
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    validator = Validator(
        repository=arguments.repository,
        target_ref=arguments.target_ref,
        max_root_header_bytes=arguments.max_root_header_bytes,
        allow_missing_test_conversions=(
            arguments.allow_missing_test_conversions
        ),
    )
    try:
        return validator.run()
    except (OSError, RuntimeError, ValueError, KeyError, json.JSONDecodeError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
