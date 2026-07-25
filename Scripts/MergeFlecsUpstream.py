#!/usr/bin/env python3
"""Safely merge upstream Flecs into the Unreal-Flecs repository."""

from __future__ import annotations

import argparse
import subprocess
import sys
from datetime import datetime
from pathlib import Path

sys.dont_write_bytecode = True

from ValidateFlecsUpstreamMerge import Validator


class UpstreamMerger:
    def __init__(
        self,
        repository: Path,
        remote: str,
        branch: str,
        rename_threshold: int,
        max_root_header_bytes: int,
        allow_missing_test_conversions: bool,
    ) -> None:
        self.repository = repository.resolve()
        self.remote = remote
        self.branch = branch
        self.target_ref = f"{remote}/{branch}"
        self.rename_threshold = rename_threshold
        self.max_root_header_bytes = max_root_header_bytes
        self.allow_missing_test_conversions = (
            allow_missing_test_conversions
        )

    def git(
        self,
        *arguments: str,
        check: bool = True,
        capture_output: bool = True,
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
            capture_output=capture_output,
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

    def ref_exists(self, reference: str) -> bool:
        return (
            self.git(
                "rev-parse",
                "--verify",
                "--quiet",
                reference,
                check=False,
            ).returncode
            == 0
        )

    def verify_repository(self) -> None:
        detected_root = Path(
            self.git_lines("rev-parse", "--show-toplevel")[0]
        ).resolve()
        if detected_root != self.repository:
            raise RuntimeError(
                f"Expected repository root '{self.repository}', but Git "
                f"reported '{detected_root}'."
            )

    def validate(self) -> int:
        if not self.ref_exists(self.target_ref):
            raise RuntimeError(
                f"Ref '{self.target_ref}' does not exist locally. "
                "Fetch it before validation."
            )
        validator = Validator(
            repository=self.repository,
            target_ref=self.target_ref,
            max_root_header_bytes=self.max_root_header_bytes,
            allow_missing_test_conversions=(
                self.allow_missing_test_conversions
            ),
        )
        return validator.run()

    def create_backup_branch(self) -> str:
        timestamp = datetime.now().strftime("%Y%m%d-%H%M%S")
        base_name = f"backup/pre-flecs-merge-{timestamp}"
        backup_name = base_name
        suffix = 2
        while self.ref_exists(f"refs/heads/{backup_name}"):
            backup_name = f"{base_name}-{suffix}"
            suffix += 1
        self.git("branch", backup_name, "HEAD")
        return backup_name

    def merge(self) -> int:
        if self.git_lines("status", "--porcelain"):
            raise RuntimeError(
                "The worktree must be clean before starting an upstream merge."
            )
        if self.ref_exists("MERGE_HEAD"):
            raise RuntimeError("A merge is already in progress.")
        if self.remote not in self.git_lines("remote"):
            raise RuntimeError(f"Git remote '{self.remote}' does not exist.")

        print(f"Fetching {self.remote} {self.branch}...")
        self.git("fetch", self.remote, self.branch, capture_output=False)
        if not self.ref_exists(self.target_ref):
            raise RuntimeError(
                f"Fetched ref '{self.target_ref}' does not exist."
            )

        backup_branch = self.create_backup_branch()
        print(f"Created backup branch {backup_branch}.")
        print(
            "Starting merge with rename threshold "
            f"{self.rename_threshold} percent..."
        )
        result = self.git(
            "merge",
            "--no-commit",
            "--no-ff",
            f"-Xfind-renames={self.rename_threshold}%",
            self.target_ref,
            check=False,
            capture_output=False,
        )
        if result.returncode:
            print(
                "Git stopped for conflict resolution. "
                f"The backup is {backup_branch}.",
                file=sys.stderr,
            )
            print(
                "Resolve conflicts, then run "
                "python Scripts/ValidateFlecsUpstreamMerge.py",
                file=sys.stderr,
            )
            print(
                "To abandon the merge, run git merge --abort.",
                file=sys.stderr,
            )
            return result.returncode

        print("Merge applied without conflicts but has not been committed.")
        validation_result = self.validate()
        if validation_result:
            return validation_result
        print("Review the staged merge and commit it when ready.")
        return 0


def percentage(value: str) -> int:
    parsed_value = int(value)
    if parsed_value < 51 or parsed_value > 100:
        raise argparse.ArgumentTypeError(
            "rename threshold must be between 51 and 100"
        )
    return parsed_value


def positive_integer(value: str) -> int:
    parsed_value = int(value)
    if parsed_value < 1:
        raise argparse.ArgumentTypeError("value must be greater than zero")
    return parsed_value


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Start or validate a guarded upstream Flecs merge."
        )
    )
    parser.add_argument(
        "--repository",
        type=Path,
        default=Path(__file__).resolve().parent.parent,
    )
    parser.add_argument("--remote", default="upstream")
    parser.add_argument("--branch", default="master")
    parser.add_argument(
        "--rename-threshold",
        type=percentage,
        default=55,
    )
    parser.add_argument(
        "--max-root-header-bytes",
        type=positive_integer,
        default=307200,
    )
    parser.add_argument("--validate-only", action="store_true")
    parser.add_argument(
        "--allow-missing-test-conversions",
        action="store_true",
    )
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    merger = UpstreamMerger(
        repository=arguments.repository,
        remote=arguments.remote,
        branch=arguments.branch,
        rename_threshold=arguments.rename_threshold,
        max_root_header_bytes=arguments.max_root_header_bytes,
        allow_missing_test_conversions=(
            arguments.allow_missing_test_conversions
        ),
    )
    try:
        merger.verify_repository()
        if arguments.validate_only:
            return merger.validate()
        return merger.merge()
    except (OSError, RuntimeError, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
