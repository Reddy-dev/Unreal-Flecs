#!/usr/bin/env python3
"""Generate, statically validate, and optionally build/run generated Flecs tests."""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path
from typing import Sequence

from flecs_test_converter import Converter, ConverterError, default_fork, resolve_from_root, validate_manifest


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(description=__doc__)
    result.add_argument("--project-root", type=Path, default=Path(__file__).resolve().parents[3])
    result.add_argument("--upstream-root", type=Path, required=True)
    result.add_argument("--fork-root", type=Path)
    result.add_argument("--output", type=Path)
    result.add_argument("--metadata", type=Path)
    result.add_argument("--inventory", type=Path)
    result.add_argument("--libclang", type=Path)
    result.add_argument("--include-dir", type=Path, action="append", default=[])
    result.add_argument("--clang-arg", action="append", default=[])
    result.add_argument("--shared-test-header", default="Bake/FlecsTestTypes.h")
    result.add_argument("--no-shared-test-header", action="store_true")
    result.add_argument("--engine-root", type=Path, default=Path(r"D:\UnrealEngine"))
    result.add_argument("--target", default="TestECSFlecsEditor")
    result.add_argument("--configuration", default="Development")
    result.add_argument("--platform", default="Win64")
    result.add_argument("--baseline-manifest", type=Path)
    result.add_argument("--static-only", action="store_true")
    result.add_argument("--skip-build", action="store_true")
    result.add_argument("--skip-runtime", action="store_true")
    return result


def run(command: list[str], cwd: Path) -> int:
    print("+", " ".join(command))
    completed = subprocess.run(command, cwd=str(cwd), check=False)
    return completed.returncode


def check_baseline(path: Path, current: dict) -> list[str]:
    try:
        baseline = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        return [f"baseline_manifest_read_error:{error}"]
    if not isinstance(baseline, dict):
        return ["baseline_manifest_is_not_object"]
    old_ids = {(item.get("source"), item.get("case")) for item in baseline.get("tests", []) if isinstance(item, dict) and isinstance(item.get("case"), str) and not item["case"].startswith("<")}
    new_ids = {(item.get("source"), item.get("case")) for item in current.get("tests", []) if isinstance(item, dict) and isinstance(item.get("case"), str) and not item["case"].startswith("<")}
    missing = sorted(old_ids - new_ids)
    return [f"baseline_test_disappeared:{source}:{case}" for source, case in missing]


def main(argv: Sequence[str] | None = None) -> int:
    options = parser().parse_args(argv)
    project_root = options.project_root.resolve()
    upstream = resolve_from_root(options.upstream_root, project_root)
    output = resolve_from_root(options.output or project_root / "Plugins" / "Unreal-Flecs" / "Source" / "FlecsLibrary" / "Tests" / "Generated", project_root)
    fork = resolve_from_root(options.fork_root, project_root) or default_fork(project_root)
    metadata = resolve_from_root(options.metadata, project_root)
    inventory = resolve_from_root(options.inventory, project_root)
    libclang = resolve_from_root(options.libclang, project_root)
    try:
        converter = Converter(
            project_root,
            upstream,
            fork,
            output,
            metadata,
            inventory,
            libclang,
            options.include_dir,
            options.clang_arg,
            None if options.no_shared_test_header else options.shared_test_header,
        )
        manifest = converter.convert()
    except ConverterError as error:
        print(f"conversion failed: {error}", file=sys.stderr)
        return 2

    manifest_path = output / "flecs_conversion_manifest.json"
    errors = validate_manifest(manifest_path, project_root)
    if options.baseline_manifest:
        baseline_path = options.baseline_manifest.resolve()
        if baseline_path == manifest_path:
            errors.append("baseline_manifest_must_be_distinct_from_output")
        else:
            errors.extend(check_baseline(baseline_path, manifest))
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 2

    if options.static_only:
        return 0

    project_file = project_root / "TestECSFlecs.uproject"
    build_bat = options.engine_root / "Engine" / "Build" / "BatchFiles" / "Build.bat"
    editor_cmd = options.engine_root / "Engine" / "Binaries" / "Win64" / "UnrealEditor-Cmd.exe"
    if not options.skip_build:
        return_code = run([
            "cmd.exe", "/c", str(build_bat), options.target, options.platform, options.configuration,
            f"-Project={project_file}", "-WaitMutex",
        ], project_root)
        if return_code:
            return return_code
    if not options.skip_runtime:
        return run([
            str(editor_cmd), str(project_file), "-unattended", "-nop4", "-nullrhi",
            '-ExecCmds=Automation RunTests FlecsLibrary.Generated; Quit',
            "-TestExit=Automation Test Queue Empty",
        ], project_root)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
