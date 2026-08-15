#!/usr/bin/env python3
"""Convert Flecs C++ tests with libclang source ranges.

The converter intentionally has no text-only function-body fallback.  Clang
discovers declarations, namespaces, records, macros, includes, and statement
ranges; text is used only to preserve the exact source ranges selected by the
AST and to scan assertion macro arguments.
"""

from __future__ import annotations

import argparse
import ast
import hashlib
import json
import logging
import os
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Iterable, Iterator, Sequence


LOG = logging.getLogger("flecs-test-converter")
SCHEMA_VERSION = 1
GENERATED_HEADER = "// FLECS TEST CONVERTER GENERATED FILE - DO NOT EDIT."
SUPPORTED_ASSERTIONS = {
    "test_assert": 1,
    "test_bool": 2,
    "test_true": 1,
    "test_false": 1,
    "test_int": 2,
    "test_uint": 2,
    "test_flt": 2,
    "test_str": 2,
    "test_null": 1,
    "test_not_null": 1,
    "test_ptr": 2,
}
PRESERVED_MACRO_EXCLUSIONS = set(SUPPORTED_ASSERTIONS) | {"flecs_static_assert"}
# `cpp.h` is the upstream Bake C++ harness. Its shared component/test types
# are provided to generated Unreal tests by Bake/FlecsTestTypes.h instead.
HARNESS_HEADERS = {"cpp.h", "test.h", "test.hpp", "flecs_test.h", "flecs_test_utils.h", "flecstestutils.h"}
# UE's global headers define a template alias named TString. Keep upstream
# test type spelling in the source while giving the generated translation unit
# a non-conflicting token name.
UE_TYPE_ALIASES = {"TString": "FlecsGeneratedTString"}
UNSUPPORTED_TOKENS = {
    "EXPECT_DEATH": "death_test_requires_expected_failure_backend",
    "ASSERT_DEATH": "death_test_requires_expected_failure_backend",
    "EXPECT_EXIT": "death_test_requires_expected_failure_backend",
    "ASSERT_EXIT": "death_test_requires_expected_failure_backend",
    "test_expect_abort": "death_test_requires_expected_failure_backend",
    "ecs_abort": "death_test_requires_expected_failure_backend",
    "abort": "death_test_requires_expected_failure_backend",
    "pthread_create": "threading_test_requires_isolated_thread_backend",
    "ecs_os_thread_new": "threading_test_requires_isolated_thread_backend",
    "ecs_os_thread_join": "threading_test_requires_isolated_thread_backend",
    "ecs_os_api": "allocator_or_platform_test_requires_custom_fixture",
    "ecs_os_set_api": "allocator_or_platform_test_requires_custom_fixture",
    "ecs_os_malloc": "allocator_or_platform_test_requires_custom_fixture",
    "ecs_os_calloc": "allocator_or_platform_test_requires_custom_fixture",
    "ecs_os_realloc": "allocator_or_platform_test_requires_custom_fixture",
    "ecs_os_free": "allocator_or_platform_test_requires_custom_fixture",
    "malloc": "allocator_or_platform_test_requires_custom_fixture",
    "calloc": "allocator_or_platform_test_requires_custom_fixture",
    "realloc": "allocator_or_platform_test_requires_custom_fixture",
    "free": "allocator_or_platform_test_requires_custom_fixture",
    "ecs_app": "application_test_requires_application_fixture",
    "ecs_init_w_args": "command_line_test_requires_command_line_fixture",
}


class ConverterError(RuntimeError):
    pass


class ClangUnavailable(ConverterError):
    pass


@dataclass(frozen=True)
class Location:
    line: int
    column: int
    offset: int | None = None

    def json(self) -> dict[str, int]:
        result = {"line": self.line, "column": self.column}
        if self.offset is not None and self.offset >= 0:
            result["offset"] = self.offset
        return result


@dataclass(frozen=True)
class Registration:
    type_name: str
    kind: str = "native"
    hook: str | None = None
    header: str | None = None

    def json(self) -> dict[str, str]:
        result = {"type": self.type_name, "kind": self.kind}
        if self.hook:
            result["hook"] = self.hook
        if self.header:
            result["header"] = self.header
        return result


@dataclass
class Metadata:
    case_id: str
    source: str | None = None
    function: str | None = None
    category: str | None = None
    tags: list[str] = field(default_factory=list)
    fixture: str = "raw_world"
    registrations: list[Registration] = field(default_factory=list)
    required_headers: list[str] = field(default_factory=list)
    registration_hooks: list[str] = field(default_factory=list)
    reset_hook: str | None = None
    setup_hook: str | None = None
    teardown_hook: str | None = None
    skip_reason: str | None = None
    expected_failure: bool = False
    ordered: bool = False
    macro_generated: bool = False
    line: int | None = None


@dataclass(frozen=True)
class ExpectedCase:
    case_id: str
    category: str | None = None
    source: str | None = None
    tags: tuple[str, ...] = ()


@dataclass
class AssertionUse:
    name: str
    status: str
    location: Location
    argument_count: int
    reason: str | None = None

    def json(self) -> dict[str, Any]:
        result: dict[str, Any] = {
            "name": self.name,
            "status": self.status,
            "argument_count": self.argument_count,
            "source_location": self.location.json(),
        }
        if self.reason:
            result["reason"] = self.reason
        return result


@dataclass
class SourceModel:
    path: Path
    text: str
    starts: list[int]
    annotations: list[Metadata]
    conditions: dict[int, list[str]]
    includes: list[tuple[str, str]]
    macros: list[str]
    byte_offsets: list[int] = field(default_factory=list)

    def __post_init__(self) -> None:
        if not self.byte_offsets:
            self.byte_offsets = [0]
            for character in self.text:
                self.byte_offsets.append(self.byte_offsets[-1] + len(character.encode("utf-8")))

    def character_offset(self, byte_offset: int) -> int:
        low, high = 0, len(self.byte_offsets)
        while low + 1 < high:
            middle = (low + high) // 2
            if self.byte_offsets[middle] <= byte_offset:
                low = middle
            else:
                high = middle
        return low

    def offset(self, line: int, column: int) -> int:
        line = max(1, min(line, len(self.starts))) - 1
        target = max(0, column - 1)
        position = self.starts[line]
        consumed = 0
        while position < len(self.text) and consumed < target and self.text[position] != "\n":
            consumed += len(self.text[position].encode("utf-8"))
            position += 1
        return position

    def location(self, offset: int, line: int | None = None, column: int | None = None) -> Location:
        if line is None or column is None:
            low, high = 0, len(self.starts)
            while low + 1 < high:
                middle = (low + high) // 2
                if self.starts[middle] <= offset:
                    low = middle
                else:
                    high = middle
            line = low + 1
            column = offset - self.starts[low] + 1
        return Location(line, column, offset)


@dataclass
class Candidate:
    name: str
    case_id: str
    category: str
    tags: list[str]
    source: Path
    location: Location
    function_range: tuple[int, int]
    body_range: tuple[int, int]
    cursor: Any
    body_cursor: Any
    translation_unit: Any
    metadata: Metadata | None = None
    conditions: list[str] = field(default_factory=list)
    assertions: list[AssertionUse] = field(default_factory=list)
    registrations: list[Registration] = field(default_factory=list)
    reasons: list[str] = field(default_factory=list)
    world_variables: list[str] = field(default_factory=list)
    world_parameter: str | None = None
    reset_hook: str | None = None
    setup_hook: str | None = None
    teardown_hook: str | None = None
    macro_generated: bool = False
    return_type: str = "void"
    namespace: str = ""

    @property
    def skipped(self) -> bool:
        return bool(self.metadata and self.metadata.skip_reason)

    @property
    def status(self) -> str:
        if self.skipped:
            return "skipped"
        return "unsupported" if self.reasons else "converted"


@dataclass
class ParsedSource:
    model: SourceModel
    translation_unit: Any
    candidates: list[Candidate]
    declarations: str
    direct_includes: list[tuple[str, str]]
    required_headers: list[str]
    macro_tests: list[tuple[str, str, Location]]
    diagnostics: list[str]
    global_reset_expressions: list[str] = field(default_factory=list)
    global_type_names: set[str] = field(default_factory=set)
    global_enum_constants: set[str] = field(default_factory=set)


def sanitize_identifier(value: str, fallback: str = "Generated") -> str:
    value = re.sub(r"[^A-Za-z0-9_]", "_", value)
    value = re.sub(r"_+", "_", value).strip("_") or fallback
    return f"_{value}" if value[0].isdigit() else value


def sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def engine_include_roots(project_root: Path) -> list[Path]:
    """Resolve the installed engine from the project's EngineAssociation."""
    project_files = sorted(project_root.glob("*.uproject"))
    if not project_files:
        return []
    try:
        descriptor = json.loads(project_files[0].read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return []
    association = descriptor.get("EngineAssociation")
    if not isinstance(association, str) or not association:
        return []

    candidates: list[Path] = []
    association_path = Path(association)
    if association_path.is_absolute():
        candidates.append(association_path)
    engine_name = association if association.startswith("UE_") else f"UE_{association}"
    for variable in ("ProgramFiles", "ProgramW6432"):
        value = os.environ.get(variable)
        if value:
            candidates.append(Path(value) / "Epic Games" / engine_name)

    for engine_root in candidates:
        core_public = engine_root / "Engine" / "Source" / "Runtime" / "Core" / "Public"
        if (core_public / "Misc" / "Build.h").is_file():
            return [
                core_public,
                engine_root / "Engine" / "Source" / "Runtime" / "CoreUObject" / "Public",
            ]
    return []


def normalize_newlines(text: str) -> str:
    return text.replace("\r\n", "\n").replace("\r", "\n")


def parser_compatibility_text(text: str) -> str:
    """Apply same-length C++ member renames to libclang's in-memory source."""
    replacements = {"ensure": "obtain", "ensure_second": "obtain_second"}

    def replace(match: re.Match[str]) -> str:
        return f"{match.group('access')}{match.group('whitespace')}{replacements[match.group('name')]}"

    return re.sub(
        r"(?P<access>\.|->)(?P<whitespace>\s*)(?P<name>ensure_second|ensure)\b",
        replace,
        text,
    )


def display_path(path: Path | None, project_root: Path) -> str | None:
    if path is None:
        return None
    try:
        return path.resolve().relative_to(project_root.resolve()).as_posix()
    except ValueError:
        return path.resolve().as_posix()


def resolve_display_path(value: str, project_root: Path) -> Path:
    path = Path(value.replace("/", os.sep))
    return path if path.is_absolute() else project_root / path


def normalized_path(value: str) -> str:
    return value.replace("\\", "/").lstrip("./").lower()


def split_top_level(value: str, delimiter: str = ",") -> list[str]:
    result: list[str] = []
    start = 0
    quote: str | None = None
    escaped = False
    depth = {"(": 0, "[": 0, "{": 0, "<": 0}
    pairs = {")": "(", "]": "[", "}": "{", ">": "<",
    }
    for index, character in enumerate(value):
        if quote:
            if escaped:
                escaped = False
            elif character == "\\":
                escaped = True
            elif character == quote:
                quote = None
            continue
        if character in {'"', "'"}:
            quote = character
        elif character in depth:
            depth[character] += 1
        elif character in pairs:
            depth[pairs[character]] = max(0, depth[pairs[character]] - 1)
        elif character == delimiter and not any(depth.values()):
            result.append(value[start:index].strip())
            start = index + 1
    result.append(value[start:].strip())
    return [item for item in result if item]


def unquote(value: str) -> str:
    value = value.strip().rstrip(")").strip()
    if len(value) >= 2 and value[0] in {'"', "'"} and value[-1] == value[0]:
        try:
            return str(ast.literal_eval(value))
        except (SyntaxError, ValueError):
            return value[1:-1]
    return value


def parse_tags(value: str | None) -> list[str]:
    if not value:
        return []
    return [unquote(item) for item in split_top_level(unquote(value).replace("|", ","))]


def parse_bool(value: Any) -> bool:
    if isinstance(value, bool):
        return value
    return str(value).lower() in {"1", "true", "yes", "on"}


def parse_metadata_payload(payload: str, line: int | None = None) -> Metadata | None:
    payload = payload.strip()
    if payload.startswith(":"):
        payload = payload[1:].strip()
    values: dict[str, str] = {}
    for item in split_top_level(payload):
        if "=" in item:
            key, value = item.split("=", 1)
            values[key.strip().lower()] = unquote(value)
    if "id" not in values and "case" not in values:
        quoted = re.findall(r"(['\"])(.*?)\1", payload)
        if quoted:
            values["id"] = quoted[0][1]
            if len(quoted) > 1:
                values["category"] = quoted[1][1]
            if len(quoted) > 2:
                values["tags"] = quoted[2][1]
    case_id = values.get("id") or values.get("case")
    if not case_id:
        return None
    registrations: list[Registration] = []
    for item in parse_tags(values.get("registrations") or values.get("registration")):
        pieces = item.split(":")
        registrations.append(Registration(pieces[0], pieces[1] if len(pieces) > 1 else "native", pieces[2] if len(pieces) > 2 else None))
    return Metadata(
        case_id=case_id,
        function=values.get("function"),
        category=values.get("category") or values.get("suite"),
        tags=parse_tags(values.get("tags")),
        fixture=values.get("fixture", "raw_world"),
        registrations=registrations,
        required_headers=parse_tags(values.get("headers") or values.get("required_headers")),
        registration_hooks=parse_tags(values.get("hooks") or values.get("registration_hooks")),
        reset_hook=values.get("reset_hook"),
        setup_hook=values.get("setup_hook"),
        teardown_hook=values.get("teardown_hook"),
        skip_reason=values.get("skip") or values.get("skip_reason"),
        expected_failure=parse_bool(values.get("expected_failure", "")),
        ordered=parse_bool(values.get("ordered", "")),
        macro_generated=parse_bool(values.get("macro_generated", "")),
        line=line,
    )


def parse_annotations(text: str) -> list[Metadata]:
    result: list[Metadata] = []
    for line_number, line in enumerate(text.splitlines(), 1):
        lower = line.lower()
        payload: str | None = None
        if "flecs-test:" in lower:
            marker = lower.index("flecs-test:") + len("flecs-test:")
            payload = line[marker:]
        elif "flecs_test_case" in lower:
            marker = lower.index("flecs_test_case") + len("flecs_test_case")
            payload = line[marker:]
        if payload:
            metadata = parse_metadata_payload(payload, line_number)
            if metadata:
                result.append(metadata)
    return result


def parse_conditions(text: str) -> dict[int, list[str]]:
    result: dict[int, list[str]] = {}
    stack: list[str] = []
    for line_number, line in enumerate(text.splitlines(), 1):
        directive = line.lstrip()
        if directive.startswith("#"):
            keyword, _, expression = directive[1:].strip().partition(" ")
            if keyword in {"if", "ifdef", "ifndef"}:
                stack.append(f"#{keyword} {expression}".strip())
            elif keyword == "elif" and stack:
                stack[-1] = f"#elif {expression}".strip()
            elif keyword == "else" and stack:
                stack[-1] = "#else"
            elif keyword == "endif" and stack:
                stack.pop()
        result[line_number] = list(stack)
    return result


def parse_includes(text: str) -> list[tuple[str, str]]:
    pattern = re.compile(r"^\s*#\s*include\s*([<\"])([^>\"]+)[>\"]")
    result: list[tuple[str, str]] = []
    for line in text.splitlines():
        match = pattern.match(line)
        if match:
            result.append((match.group(1), match.group(2).strip()))
    return result


def parse_macro_definitions(text: str) -> list[str]:
    lines = text.splitlines()
    result: list[str] = []
    index = 0
    while index < len(lines):
        if lines[index].lstrip().startswith("#define"):
            block = [lines[index]]
            while block[-1].rstrip().endswith("\\") and index + 1 < len(lines):
                index += 1
                block.append(lines[index])
            result.append("\n".join(block))
        index += 1
    return result


def build_model(path: Path | str, text: str) -> SourceModel:
    path = Path(path)
    starts = [0]
    starts.extend(index + 1 for index, value in enumerate(text) if value == "\n")
    return SourceModel(path, text, starts, parse_annotations(text), parse_conditions(text), parse_includes(text), parse_macro_definitions(text))


def load_metadata(path: Path | None) -> list[Metadata]:
    if path is None:
        return []
    try:
        document = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise ConverterError(f"Could not read metadata {path}: {error}") from error
    if isinstance(document, list):
        records = document
    elif isinstance(document, dict):
        records = document.get("tests", document.get("cases", []))
    else:
        raise ConverterError(f"Metadata document must be an object or array: {path}")
    if not isinstance(records, list):
        raise ConverterError(f"Metadata tests must be an array: {path}")
    result: list[Metadata] = []
    for record in records:
        if not isinstance(record, dict) or not record.get("id", record.get("case")):
            raise ConverterError(f"Metadata entry is missing id/case: {path}")
        tags = record.get("tags", [])
        required_headers = record.get("required_headers", record.get("headers", []))
        registration_hooks = record.get("registration_hooks", record.get("hooks", []))
        if isinstance(tags, str):
            tags = parse_tags(tags)
        if isinstance(required_headers, str):
            required_headers = [required_headers]
        if isinstance(registration_hooks, str):
            registration_hooks = [registration_hooks]
        registrations: list[Registration] = []
        registration_values = record.get("registrations", record.get("registration", []))
        if isinstance(registration_values, str):
            registration_values = [registration_values]
        for item in registration_values:
            if isinstance(item, str):
                registrations.append(Registration(item))
            elif isinstance(item, dict) and item.get("type"):
                registrations.append(Registration(str(item["type"]), str(item.get("kind", "native")), item.get("hook"), item.get("header")))
        result.append(Metadata(
            case_id=str(record.get("id", record.get("case"))),
            source=record.get("source"),
            function=record.get("function"),
            category=record.get("category", record.get("suite")),
            tags=[str(item) for item in tags],
            fixture=str(record.get("fixture", "raw_world")),
            registrations=registrations,
            required_headers=[str(item) for item in required_headers],
            registration_hooks=[str(item) for item in registration_hooks],
            reset_hook=record.get("reset_hook"),
            setup_hook=record.get("setup_hook"),
            teardown_hook=record.get("teardown_hook"),
            skip_reason=record.get("skip_reason", record.get("skip")),
            expected_failure=parse_bool(record.get("expected_failure", False)),
            ordered=parse_bool(record.get("ordered", False)),
            macro_generated=parse_bool(record.get("macro_generated", False)),
        ))
    return result


def load_inventory(path: Path | None) -> list[ExpectedCase]:
    if path is None:
        return []
    try:
        document = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise ConverterError(f"Could not read inventory {path}: {error}") from error
    test_document = document.get("test", {}) if isinstance(document, dict) else {}
    suites = test_document.get("testsuites", []) if isinstance(test_document, dict) else []
    if not isinstance(suites, list):
        raise ConverterError(f"Inventory test suites must be an array: {path}")
    result: list[ExpectedCase] = []
    for suite in suites:
        if not isinstance(suite, dict) or not suite.get("id"):
            continue
        category = str(suite["id"])
        suite_source = suite.get("source", suite.get("file"))
        cases = suite.get("testcases", [])
        if not isinstance(cases, list):
            raise ConverterError(f"Inventory testcases must be an array: {path}")
        for case in cases:
            case_source = suite_source
            if isinstance(case, dict):
                case_source = case.get("source", case.get("file", suite_source))
                case = case.get("id", case.get("name", ""))
            case = str(case)
            result.append(ExpectedCase(case if case.startswith(category + "_") else f"{category}_{case}", category, case_source))
    return result


def kind(cursor: Any) -> str:
    return str(getattr(cursor, "kind", "")).split(".")[-1]


def walk(cursor: Any) -> Iterator[Any]:
    for child in cursor.get_children():
        yield child
        yield from walk(child)


def walk_source(cursor: Any, path: Path) -> Iterator[Any]:
    """Walk only declarations spelled in one source file, pruning includes."""
    for child in cursor.get_children():
        if not same_source(child, path):
            continue
        yield child
        yield from walk_source(child, path)


def same_source(cursor: Any, path: Path) -> bool:
    file = getattr(getattr(cursor, "location", None), "file", None)
    if file is None or not getattr(file, "name", None):
        return False
    # This predicate is called for every declaration exposed by libclang.
    # Path.resolve() performs filesystem work and turns a single large test
    # source into minutes of path lookups on Windows.
    return os.path.normcase(os.path.normpath(str(file.name))) == os.path.normcase(os.path.normpath(str(path)))


def cursor_offset(location: Any, model: SourceModel) -> int:
    offset = getattr(location, "offset", -1)
    if isinstance(offset, int) and offset >= 0:
        return model.character_offset(offset)
    return model.offset(int(getattr(location, "line", 1)), int(getattr(location, "column", 1)))


def cursor_range(cursor: Any, model: SourceModel) -> tuple[int, int]:
    return cursor_offset(cursor.extent.start, model), cursor_offset(cursor.extent.end, model)


def cursor_location(cursor: Any, model: SourceModel) -> Location:
    return Location(int(getattr(cursor.location, "line", 1)), int(getattr(cursor.location, "column", 1)), cursor_offset(cursor.location, model))


def body_cursor(cursor: Any) -> Any | None:
    return next((child for child in cursor.get_children() if kind(child) == "COMPOUND_STMT"), None)


def parent_kind(cursor: Any) -> str:
    return kind(getattr(cursor, "semantic_parent", None))


def metadata_for(model: SourceModel, function: str, line: int, external: Sequence[Metadata], path: Path, roots: Sequence[Path]) -> Metadata | None:
    def matches(item: Metadata) -> bool:
        if item.source:
            values = {normalized_path(path.as_posix()), normalized_path(path.name)}
            for root in roots:
                try:
                    values.add(normalized_path(path.resolve().relative_to(root.resolve()).as_posix()))
                except ValueError:
                    pass
            source = normalized_path(item.source)
            if source not in values and not any(value.endswith(source) for value in values):
                return False
        return True

    exact = [item for item in external if matches(item) and (item.function == function or item.case_id == function)]
    if exact:
        return exact[-1]
    suffix = [item for item in external if matches(item) and item.case_id.endswith("_" + function)]
    if suffix:
        return suffix[-1]
    annotations = [item for item in model.annotations if item.line and item.line <= line]
    if annotations and line - (annotations[-1].line or line) <= 4:
        return annotations[-1]
    return None


def likely_test_name(name: str, source_stem: str) -> bool:
    if name in {"main", "Define"} or name in SUPPORTED_ASSERTIONS or name.startswith("operator"):
        return False
    return name.startswith("test_") or name.startswith(f"{source_stem}_")


def expected_for(function: str, inventory: Sequence[ExpectedCase]) -> ExpectedCase | None:
    exact = [item for item in inventory if item.case_id == function]
    suffix = [item for item in inventory if item.case_id.endswith("_" + function)]
    return (exact or suffix)[0] if exact or suffix else None


def token_location(token: Any, model: SourceModel) -> Location:
    return Location(int(getattr(token.location, "line", 1)), int(getattr(token.location, "column", 1)), cursor_offset(token.location, model))


def macro_arguments(tokens: Sequence[Any], open_index: int) -> tuple[int, int]:
    parentheses = 0
    brackets = 0
    braces = 0
    angles = 0
    count = 0
    has_value = False
    for index in range(open_index + 1, len(tokens)):
        spelling = tokens[index].spelling
        if spelling == "(":
            parentheses += 1
            has_value = True
        elif spelling == ")":
            if parentheses == 0:
                if has_value or count:
                    count += 1
                return count, index
            parentheses -= 1
        elif spelling == "[":
            brackets += 1
            has_value = True
        elif spelling == "]":
            brackets = max(0, brackets - 1)
        elif spelling == "{":
            braces += 1
            has_value = True
        elif spelling == "}":
            braces = max(0, braces - 1)
        elif spelling == "<":
            previous = tokens[index - 1].spelling if index > open_index + 1 else ""
            next_spelling = tokens[index + 1].spelling if index + 1 < len(tokens) else ""
            if (previous == ">" or previous == "::" or re.match(r"^[A-Za-z_]\w*$", previous)) and (next_spelling == ">" or next_spelling == "::" or re.match(r"^[A-Za-z_]\w*$", next_spelling)):
                angles += 1
            else:
                has_value = True
        elif spelling == ">" and angles:
            angles -= 1
        elif spelling == "," and not any((parentheses, brackets, braces, angles)):
            count += 1
            has_value = False
        else:
            has_value = True
    return count, len(tokens) - 1


def scan_assertions(tokens: Sequence[Any], model: SourceModel) -> list[AssertionUse]:
    result: list[AssertionUse] = []
    index = 0
    while index + 1 < len(tokens):
        name = tokens[index].spelling
        if name.startswith("test_") and tokens[index + 1].spelling == "(":
            count, end = macro_arguments(tokens, index + 1)
            location = token_location(tokens[index], model)
            expected = SUPPORTED_ASSERTIONS.get(name)
            if expected is None:
                result.append(AssertionUse(name, "unsupported", location, count, f"unsupported_assertion:{name}"))
            elif expected != count:
                result.append(AssertionUse(name, "unsupported", location, count, f"assertion_argument_count:{name}"))
            else:
                result.append(AssertionUse(name, "supported", location, count))
            index = max(index + 1, end + 1)
        else:
            index += 1
    return result


def split_template_arguments(tokens: Sequence[Any], start: int) -> tuple[list[str], int]:
    values: list[str] = []
    current: list[str] = []
    depth = 0
    for index in range(start + 1, len(tokens)):
        spelling = tokens[index].spelling
        if spelling == "<":
            depth += 1
            current.append(spelling)
        elif spelling in {">", ">>", ">>>"}:
            for _ in range(len(spelling)):
                if depth == 0:
                    if current:
                        values.append("".join(current))
                    return [clean_type(value) for value in values], index
                depth -= 1
                current.append(">")
        elif spelling == "," and depth == 0:
            values.append("".join(current))
            current = []
        else:
            current.append(spelling)
    return [clean_type(value) for value in values], len(tokens) - 1


def clean_type(value: str) -> str:
    value = re.sub(r"\b(?:const|volatile|typename|class|struct)\b", "", value)
    # libclang token spellings can omit whitespace when a qualifier is the
    # first token in a template argument (for example ``const Velocity`` may
    # arrive as ``constVelocity``). Do not turn that into a new type name.
    value = re.sub(r"^(?:const|volatile)(?=[A-Za-z_])", "", value)
    return value.replace("&", "").replace("*", "").strip()


def infer_types(tokens: Sequence[Any], namespace: str) -> list[str]:
    methods = {"component", "entity", "query", "query_builder", "has", "get", "try_get", "get_mut", "try_get_mut", "set", "emplace", "ensure", "add", "remove", "system", "observer", "observe", "singleton"}
    result: list[str] = []
    index = 0
    while index + 1 < len(tokens):
        if tokens[index].spelling in methods and tokens[index + 1].spelling == "<":
            values, end = split_template_arguments(tokens, index + 1)
            for value in values:
                if not value or value.startswith("flecs::") or value.startswith("std::") or value in {"int", "float", "double", "bool", "void"}:
                    continue
                if any(character in value for character in "()=;{}"):
                    continue
                if namespace and "::" not in value and re.match(r"^[A-Za-z_]\w*$", value):
                    value = f"{namespace}::{value}"
                if value not in result:
                    result.append(value)
            index = max(index + 1, end + 1)
        else:
            index += 1
    return result


def namespace_name(cursor: Any) -> str:
    names: list[str] = []
    current = getattr(cursor, "semantic_parent", None)
    while current is not None:
        if kind(current) == "NAMESPACE" and getattr(current, "spelling", ""):
            names.append(current.spelling)
        current = getattr(current, "semantic_parent", None)
    return "::".join(reversed(names))


def world_type(spelling: str) -> bool:
    return spelling.replace(" ", "") in {"flecs::world", "flecs::world&", "classflecs::world", "classflecs::world&"}


def world_variables(candidate: Candidate, model: SourceModel) -> list[str]:
    result: list[str] = []
    for child in walk(candidate.cursor):
        if kind(child) != "VAR_DECL" or not same_source(child, model.path):
            continue
        start, end = cursor_range(child, model)
        if start < candidate.body_range[0] or end > candidate.body_range[1]:
            continue
        spelling = getattr(getattr(child, "type", None), "spelling", "")
        if world_type(spelling):
            if getattr(child, "spelling", ""):
                result.append(child.spelling)
    return list(dict.fromkeys(result))


def world_parameter(cursor: Any) -> str | None:
    try:
        arguments = list(cursor.get_arguments())
    except AttributeError:
        arguments = [child for child in cursor.get_children() if kind(child) == "PARM_DECL"]
    if not arguments:
        return None
    for argument in arguments:
        if not world_type(getattr(getattr(argument, "type", None), "spelling", "")):
            return "__invalid__"
    return getattr(arguments[0], "spelling", "world") if len(arguments) == 1 else "__invalid__"


def unsupported_constructs(tokens: Sequence[Any]) -> list[str]:
    spellings = [token.spelling for token in tokens]
    reasons: list[str] = []
    for spelling in spellings:
        if spelling in UNSUPPORTED_TOKENS:
            reasons.append(UNSUPPORTED_TOKENS[spelling])
        if spelling.startswith("pthread_"):
            reasons.append("threading_test_requires_isolated_thread_backend")
    for index, spelling in enumerate(spellings[:-2]):
        if spelling == "std" and spellings[index + 1] == "::" and f"std::{spellings[index + 2]}" in {"std::thread", "std::async", "std::mutex", "std::condition_variable", "std::future", "std::jthread"}:
            reasons.append("threading_test_requires_isolated_thread_backend")
    if "argc" in spellings and "argv" in spellings:
        reasons.append("command_line_test_requires_command_line_fixture")
    return list(dict.fromkeys(reasons))


def source_reflected_types(tu: Any, model: SourceModel) -> set[str]:
    result: set[str] = set()
    for cursor in walk_source(tu.cursor, model.path):
        if kind(cursor) not in {"STRUCT_DECL", "CLASS_DECL", "ENUM_DECL"}:
            continue
        start, _ = cursor_range(cursor, model)
        if re.search(r"\b(?:USTRUCT|UCLASS|UENUM)\s*\(", model.text[max(0, start - 160):start]):
            result.add(getattr(cursor, "spelling", ""))
    return result


def global_mutable_state_expressions(tu: Any, model: SourceModel) -> list[str]:
    """Return assignable names for mutable variables with file lifetime."""
    result: list[str] = []
    for cursor in walk_source(tu.cursor, model.path):
        if kind(cursor) != "VAR_DECL":
            continue
        if kind(getattr(cursor, "semantic_parent", None)) not in {"TRANSLATION_UNIT", "NAMESPACE"}:
            continue
        start, end = cursor_range(cursor, model)
        declaration = model.text[start:end]
        if "constexpr" in declaration or "const " in declaration or "consteval" in declaration:
            continue
        if "static" not in declaration and kind(getattr(cursor, "semantic_parent", None)) != "TRANSLATION_UNIT":
            continue
        name = getattr(cursor, "spelling", "")
        if not name:
            continue
        # Use the written qualified name (for example Pod::ctor_invoked), not
        # Cursor.spelling, which drops the enclosing type for static members.
        match = re.search(rf"(?P<name>(?:\b[A-Za-z_]\w*::)*\b{re.escape(name)})\s*(?:=|;|\{{)", declaration)
        expression = match.group("name") if match else name
        namespace = namespace_name(cursor)
        if namespace and "::" not in expression:
            expression = f"::{namespace}::{expression}"
        if expression not in result:
            result.append(expression)
    return result


def has_static_local_state(candidate: Candidate, model: SourceModel) -> bool:
    for cursor in walk(candidate.cursor):
        if kind(cursor) != "VAR_DECL" or not same_source(cursor, model.path):
            continue
        start, end = cursor_range(cursor, model)
        if start < candidate.body_range[0] or end > candidate.body_range[1]:
            continue
        declaration = model.text[start:end]
        if re.search(r"\bstatic\b", declaration) and "const " not in declaration and "constexpr" not in declaration:
            return True
    return False


def has_world_pointer(candidate: Candidate, model: SourceModel) -> bool:
    for cursor in walk(candidate.cursor):
        if kind(cursor) != "VAR_DECL" or not same_source(cursor, model.path):
            continue
        start, end = cursor_range(cursor, model)
        if start < candidate.body_range[0] or end > candidate.body_range[1]:
            continue
        spelling = getattr(getattr(cursor, "type", None), "spelling", "").replace(" ", "")
        if "flecs::world*" in spelling or "classflecs::world*" in spelling:
            return True
    return False


def analyze(candidate: Candidate, model: SourceModel, reflected_types: set[str]) -> None:
    tokens = list(candidate.translation_unit.get_tokens(extent=candidate.body_cursor.extent))
    candidate.assertions = scan_assertions(tokens, model)
    candidate.reasons.extend(item.reason for item in candidate.assertions if item.reason)
    candidate.reasons.extend(unsupported_constructs(tokens))
    if any(re.search(r"\b(?:_WIN32|_WIN64|__linux__|__APPLE__|__ANDROID__|PLATFORM_[A-Za-z0-9_]+)\b", condition) for condition in candidate.conditions):
        candidate.reasons.append("platform_specific_test_requires_platform_fixture")
    if candidate.macro_generated:
        candidate.reasons.append("macro_generated_test_requires_compiler_expansion")
    if candidate.return_type not in {"", "void"}:
        candidate.reasons.append("test_function_must_return_void")
    candidate.registrations = list(candidate.metadata.registrations) if candidate.metadata else []
    known = {item.type_name for item in candidate.registrations}
    for type_name in infer_types(tokens, namespace_name(candidate.cursor)):
        if type_name not in known:
            candidate.registrations.append(Registration(type_name))
            known.add(type_name)
    if candidate.metadata:
        candidate.reset_hook = candidate.metadata.reset_hook
        candidate.setup_hook = candidate.metadata.setup_hook
        candidate.teardown_hook = candidate.metadata.teardown_hook
        if candidate.metadata.expected_failure:
            candidate.reasons.append("expected_failure_test_requires_explicit_backend")
        if candidate.metadata.ordered:
            candidate.reasons.append("ordered_global_setup_requires_ordered_fixture")
        if candidate.metadata.fixture != "raw_world":
            candidate.reasons.append(f"unsupported_fixture:{candidate.metadata.fixture}")
    if has_static_local_state(candidate, model) and not candidate.reset_hook:
        candidate.reasons.append("static_local_state_requires_reset_hook")
    if has_world_pointer(candidate, model):
        candidate.reasons.append("pointer_world_requires_adapter")
    if candidate.world_parameter == "__invalid__":
        candidate.reasons.append("test_function_has_non_world_parameters")
    for registration in candidate.registrations:
        if registration.kind not in {"native", "raw_world", "unreal", "uobject", "ustruct", "uclass", "fork"}:
            candidate.reasons.append(f"unsupported_registration_kind:{registration.kind}")
        if registration.kind in {"unreal", "uobject", "ustruct", "uclass", "fork"} and not registration.hook:
            candidate.reasons.append(f"registration_hook_missing:{registration.type_name}")
        if registration.type_name.split("::")[-1] in reflected_types and not registration.hook:
            candidate.reasons.append(f"reflected_type_requires_registration_hook:{registration.type_name}")
    candidate.reasons = list(dict.fromkeys(candidate.reasons))


def macro_tests(model: SourceModel) -> list[tuple[str, str, Location]]:
    result: list[tuple[str, str, Location]] = []
    pattern = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*TEST[A-Za-z0-9_]*)\s*\(")
    for line_number, line in enumerate(model.text.splitlines(), 1):
        if line.lstrip().startswith("#define"):
            continue
        match = pattern.search(line)
        if not match:
            continue
        macro = match.group(1)
        if not line.lstrip().startswith(macro):
            continue
        if not (macro.startswith("FLECS") or macro.startswith("ECS") or macro.startswith("TEST")):
            continue
        quoted = re.search(r"(['\"])(.*?)\1", line[match.end():])
        annotations = [item for item in model.annotations if item.line and item.line <= line_number and line_number - item.line <= 4]
        case_id = annotations[-1].case_id if annotations else quoted.group(2) if quoted else f"{macro}_{line_number}"
        result.append((case_id, macro, Location(line_number, match.start(1) + 1)))
    return result


def shared_test_type_names(shared_header: str | None, translation_unit: Any | None = None) -> set[str]:
    if not shared_header:
        return set()
    header = Path(__file__).resolve().parent.parent / "Source" / "FlecsLibrary" / "Tests" / shared_header
    try:
        text = header.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return set()
    # This header is parsed for every upstream source. Keep this operation
    # lexical and bounded; walking every included UE/Flecs cursor for every
    # source makes conversion needlessly quadratic. Named namespace/type
    # braces are enough for the compatibility header and preserve nested
    # names such as Pod::Child without treating a source-local Foo as global.
    text = re.sub(r"//[^\n]*|/\*.*?\*/", "", text, flags=re.DOTALL)
    declaration_pattern = re.compile(r"\b(namespace|class|struct|enum)(?:\s+(?:class|struct))?\s+([A-Za-z_]\w*)")
    alias_pattern = re.compile(r"\b(?:using|typedef)\s+([A-Za-z_]\w*)\s*(?:=|;)")
    events: dict[int, tuple[str, str]] = {}
    for match in declaration_pattern.finditer(text):
        opening = text.find("{", match.end())
        if opening != -1:
            events[opening] = (match.group(1), match.group(2))
    names: set[str] = set()
    scopes: list[str | None] = []
    pending_aliases: list[tuple[int, str]] = []
    for match in alias_pattern.finditer(text):
        pending_aliases.append((match.start(), match.group(1)))
    alias_index = 0
    for position, character in enumerate(text):
        while alias_index < len(pending_aliases) and pending_aliases[alias_index][0] <= position:
            alias_name = pending_aliases[alias_index][1]
            prefix = [item for item in scopes if item]
            names.add("::".join(prefix + [alias_name]))
            alias_index += 1
        if character == "{":
            declaration = events.get(position)
            scopes.append(declaration[1] if declaration else None)
            if declaration:
                prefix = [item for item in scopes[:-1] if item]
                names.add("::".join(prefix + [declaration[1]]))
        elif character == "}" and scopes:
            scopes.pop()
    return names


def qualified_type_name(cursor: Any) -> str:
    name = getattr(cursor, "spelling", "")
    if not name or name.startswith("<"):
        return ""
    parts = [name]
    parent = getattr(cursor, "semantic_parent", None)
    while parent is not None:
        if kind(parent) in {"NAMESPACE", "STRUCT_DECL", "CLASS_DECL", "CLASS_TEMPLATE"}:
            spelling = getattr(parent, "spelling", "")
            if spelling and not spelling.startswith("<"):
                parts.append(spelling)
        parent = getattr(parent, "semantic_parent", None)
    return "::".join(reversed(parts))


def source_global_type_names(tu: Any, model: SourceModel, shared_types: set[str]) -> set[str]:
    """Return source/header types that are visible outside a test function."""
    result = set(shared_types)
    declaration_kinds = {
        "STRUCT_DECL",
        "CLASS_DECL",
        "CLASS_TEMPLATE",
        "ENUM_DECL",
        "TYPEDEF_DECL",
        "TYPE_ALIAS_DECL",
    }
    for cursor in walk_source(tu.cursor, model.path):
        if kind(cursor) not in declaration_kinds:
            continue
        parent = getattr(cursor, "semantic_parent", None)
        local = False
        while parent is not None:
            if kind(parent) in {"FUNCTION_DECL", "FUNCTION_TEMPLATE", "CXX_METHOD", "CONSTRUCTOR", "DESTRUCTOR"}:
                local = True
                break
            parent = getattr(parent, "semantic_parent", None)
        if not local:
            name = qualified_type_name(cursor)
            if name:
                result.add(name)
                if "::" not in name:
                    result.add(name)
    return result


def source_global_enum_constants(tu: Any, model: SourceModel) -> set[str]:
    """Return unscoped enum values declared at file/namespace scope.

    Generated tests are compiled by Unreal's unity build. Unscoped enum values
    therefore share a preprocessor/translation-unit namespace even when their
    original tests lived in separate upstream files. The renderer gives these
    values source-unique aliases before preserving the declarations.
    """
    result: set[str] = set()
    for cursor in walk_source(tu.cursor, model.path):
        if kind(cursor) != "ENUM_CONSTANT_DECL":
            continue
        enum_cursor = getattr(cursor, "semantic_parent", None)
        if enum_cursor is None or kind(enum_cursor) != "ENUM_DECL":
            continue
        parent = getattr(enum_cursor, "semantic_parent", None)
        local_or_class_scope = False
        while parent is not None:
            parent_kind_name = kind(parent)
            if parent_kind_name in {"FUNCTION_DECL", "FUNCTION_TEMPLATE", "CXX_METHOD", "CONSTRUCTOR", "DESTRUCTOR"}:
                local_or_class_scope = True
                break
            if parent_kind_name in {"STRUCT_DECL", "CLASS_DECL", "CLASS_TEMPLATE"}:
                local_or_class_scope = True
                break
            parent = getattr(parent, "semantic_parent", None)
        if local_or_class_scope:
            continue
        scoped = getattr(enum_cursor, "is_scoped_enum", None)
        if callable(scoped):
            try:
                if scoped():
                    continue
            except Exception:
                pass
        enum_start, enum_end = cursor_range(enum_cursor, model)
        enum_prefix = model.text[enum_start:min(enum_end, enum_start + 96)]
        if re.search(r"\benum\s+(?:class|struct)\b", enum_prefix):
            continue
        name = getattr(cursor, "spelling", "")
        if name:
            result.add(name)
    return result


def registration_type_is_global(type_name: str, global_types: set[str]) -> bool:
    """Check that every user type in a registration is file-visible."""
    normalized = clean_type(type_name)
    if not normalized or normalized.startswith("flecs::") or normalized.startswith("std::"):
        return False
    scalar_types = {
        "bool", "char", "signed char", "unsigned char", "short", "unsigned short",
        "int", "unsigned int", "long", "unsigned long", "long long", "unsigned long long",
        "int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t",
        "int64_t", "uint64_t", "intptr_t", "uintptr_t", "size_t", "float", "double",
    }
    if normalized in scalar_types:
        return True
    if normalized in global_types:
        return True
    identifiers = re.findall(r"(?:[A-Za-z_]\w*::)*[A-Za-z_]\w*", normalized)
    if not identifiers:
        return False
    for identifier in identifiers:
        if identifier in {"const", "volatile", "typename"}:
            continue
        if identifier not in global_types and identifier.split("::")[-1] not in global_types:
            return False
    return True


def preserved_declarations(tu: Any, model: SourceModel, candidates: Sequence[Candidate], shared_types: set[str]) -> str:
    excluded = [candidate.function_range for candidate in candidates]
    semicolon_terminated = {"STRUCT_DECL", "CLASS_DECL", "CLASS_TEMPLATE", "ENUM_DECL", "TYPEDEF_DECL", "TYPE_ALIAS_DECL", "VAR_DECL"}
    pieces: list[str] = []
    seen_source_ranges: set[tuple[int, int]] = set()
    typedef_ranges: list[tuple[int, int]] = []
    for declaration_cursor in tu.cursor.get_children():
        if not same_source(declaration_cursor, model.path) or kind(declaration_cursor) != "TYPEDEF_DECL":
            continue
        typedef_start, typedef_end = cursor_range(declaration_cursor, model)
        if re.search(r"\b(?:struct|class|enum)\b", model.text[typedef_start:typedef_end]):
            typedef_ranges.append((typedef_start, typedef_end))
    for cursor in tu.cursor.get_children():
        if not same_source(cursor, model.path) or kind(cursor) in {"INCLUSION_DIRECTIVE", "MACRO_DEFINITION"}:
            continue
        start, end = cursor_range(cursor, model)
        if end <= start:
            continue
        if kind(cursor) == "FUNCTION_DECL" and any(start == left for left, _ in excluded):
            continue
        if kind(cursor) not in {"NAMESPACE", "STRUCT_DECL", "CLASS_DECL", "CLASS_TEMPLATE", "ENUM_DECL", "TYPEDEF_DECL", "TYPE_ALIAS_DECL", "VAR_DECL", "FUNCTION_DECL", "FUNCTION_TEMPLATE", "UNEXPOSED_DECL"}:
            continue
        source_range = (start, end)
        if source_range in seen_source_ranges:
            continue
        if kind(cursor) in {"STRUCT_DECL", "CLASS_DECL", "CLASS_TEMPLATE", "ENUM_DECL"} and any(
            typedef_start <= start and end <= typedef_end for typedef_start, typedef_end in typedef_ranges
        ):
            continue
        if kind(cursor) in {"STRUCT_DECL", "CLASS_DECL", "CLASS_TEMPLATE", "ENUM_DECL", "TYPEDEF_DECL", "TYPE_ALIAS_DECL"}:
            # FlecsTestTypes.h supplies the shared upstream harness records.
            # Do not copy a second definition into the unity-built generated
            # source when an upstream file repeats one of those records.
            declaration_name = qualified_type_name(cursor)
            if declaration_name in shared_types:
                continue
        if kind(cursor) == "VAR_DECL":
            declaration = model.text[start:end]
            owner = re.match(r"\s*[^;=]+?\b([A-Za-z_]\w*)::[A-Za-z_]\w*\s*(?:=|;|$)", declaration)
            if owner and owner.group(1) in shared_types:
                continue
        is_meta_type_macro = re.match(r"\s*ECS_(?:STRUCT|ENUM|BITMASK)\s*\(", model.text[start:end]) is not None
        if kind(cursor) in semicolon_terminated or is_meta_type_macro:
            semicolon = end
            while semicolon < len(model.text) and model.text[semicolon].isspace():
                semicolon += 1
            if semicolon < len(model.text) and model.text[semicolon] == ";":
                end = semicolon + 1
        text = model.text[start:end]
        cuts = sorted((max(0, left - start), min(end - start, right - start)) for left, right in excluded if left < end and right > start)
        if cuts:
            position = 0
            chunks: list[str] = []
            for left, right in cuts:
                chunks.append(text[position:left])
                position = max(position, right)
            chunks.append(text[position:])
            text = "".join(chunks)
        if text.strip():
            pieces.append(text.strip("\n"))
            seen_source_ranges.add(source_range)
    return "\n\n".join(pieces)


class Frontend:
    def __init__(self, library: Path | None) -> None:
        try:
            import clang.cindex as cindex  # type: ignore
        except ImportError as error:
            raise ClangUnavailable("Install the clang Python package and set CLANG_LIBRARY_FILE or --libclang.") from error
        self.cindex = cindex
        if library:
            try:
                cindex.Config.set_library_file(str(library))
            except Exception as error:
                raise ClangUnavailable(f"Could not configure libclang {library}: {error}") from error
        try:
            self.index = cindex.Index.create()
        except Exception as error:
            raise ClangUnavailable(f"Could not load libclang: {error}") from error

    def parse(self, path: Path, arguments: Sequence[str], source_text: str | None = None) -> Any:
        options = getattr(self.cindex.TranslationUnit, "PARSE_DETAILED_PROCESSING_RECORD", 0)
        options |= getattr(self.cindex.TranslationUnit, "PARSE_INCOMPLETE", 0)
        unsaved_files = [(str(path), source_text)] if source_text is not None else None
        return self.index.parse(str(path), args=list(arguments), unsaved_files=unsaved_files, options=options)


class SourceParser:
    def __init__(self, frontend: Frontend, path: Path, arguments: Sequence[str], external_metadata: Sequence[Metadata], inventory: Sequence[ExpectedCase], roots: Sequence[Path], shared_header: str | None) -> None:
        self.frontend = frontend
        self.path = path
        self.arguments = arguments
        self.external_metadata = external_metadata
        self.inventory = inventory
        self.roots = roots
        self.shared_header = shared_header

    def parse(self) -> ParsedSource:
        with self.path.open("r", encoding="utf-8", errors="replace", newline="") as source_file:
            text = source_file.read()
        model = build_model(self.path, text)
        tu = self.frontend.parse(self.path, self.arguments, parser_compatibility_text(text))
        diagnostics = [str(item) for item in tu.diagnostics]
        fatal_diagnostics = [
            str(item)
            for item in tu.diagnostics
            if int(getattr(item, "severity", 0)) >= 4
            and "too many errors emitted" not in str(item)
        ]
        if fatal_diagnostics:
            raise ConverterError("clang_parse_error:" + " | ".join(fatal_diagnostics))
        source_diagnostics: list[tuple[int, str]] = []
        for diagnostic in tu.diagnostics:
            if int(getattr(diagnostic, "severity", 0)) < 3:
                continue
            location = getattr(diagnostic, "location", None)
            diagnostic_file = getattr(location, "file", None)
            if diagnostic_file is None or Path(str(diagnostic_file)).resolve() != self.path.resolve():
                continue
            line = int(getattr(location, "line", 0))
            if line:
                source_diagnostics.append((line, f"clang_error:{getattr(diagnostic, 'spelling', str(diagnostic))}"))
        global_reset_expressions = global_mutable_state_expressions(tu, model)
        reflected_types = source_reflected_types(tu, model)
        macro_locations = {location.line: case_id for case_id, _, location in macro_tests(model)}
        candidates: list[Candidate] = []
        for cursor in walk_source(tu.cursor, self.path):
            if kind(cursor) != "FUNCTION_DECL" or not getattr(cursor, "is_definition", lambda: True)():
                continue
            if parent_kind(cursor) in {"STRUCT_DECL", "CLASS_DECL", "CLASS_TEMPLATE"}:
                continue
            body = body_cursor(cursor)
            if body is None:
                continue
            name = getattr(cursor, "spelling", "")
            location = cursor_location(cursor, model)
            metadata = metadata_for(model, name, location.line, self.external_metadata, self.path, self.roots)
            expected = expected_for(name, self.inventory)
            body_tokens = list(tu.get_tokens(extent=body.extent))
            has_test_assertion = any(token.spelling.startswith("test_") for token in body_tokens)
            assertion_test = (
                has_test_assertion
                and world_parameter(cursor) != "__invalid__"
                and "STATIC" not in str(getattr(cursor, "storage_class", "")).upper()
            )
            if not metadata and not expected and not likely_test_name(name, self.path.stem) and not assertion_test:
                continue
            macro_case = macro_locations.get(location.line)
            case_id = metadata.case_id if metadata else expected.case_id if expected else macro_case or name
            category = metadata.category if metadata and metadata.category else expected.category if expected and expected.category else case_id.split("_", 1)[0]
            body_range = cursor_range(body, model)
            candidate = Candidate(name, case_id, category or "Flecs", list(metadata.tags if metadata else expected.tags if expected else ()), self.path, location, cursor_range(cursor, model), body_range, cursor, body, tu, metadata, model.conditions.get(location.line, []), namespace=namespace_name(cursor))
            candidate.world_parameter = world_parameter(cursor)
            candidate.return_type = getattr(getattr(cursor, "result_type", None), "spelling", "void")
            candidate.world_variables = world_variables(candidate, model)
            candidate.macro_generated = bool(metadata and metadata.macro_generated) or macro_case is not None
            analyze(candidate, model, reflected_types)
            rewritten_ensure_lines = fork_cpp_api_rewrite_lines(candidate, model)
            end_line = int(getattr(body.extent.end, "line", location.line))
            candidate.reasons.extend(
                message
                for line, message in source_diagnostics
                if location.line <= line <= end_line
                and not (
                    line in rewritten_ensure_lines
                    and "no member named" in message
                    and ("ensure" in message or "ensure_second" in message)
                )
            )
            if self.roots and len(self.roots) > 1 and self.roots[1].resolve() in self.path.resolve().parents and metadata is None:
                candidate.reasons.append("fork_test_missing_explicit_metadata")
            candidate.reasons = list(dict.fromkeys(candidate.reasons))
            candidates.append(candidate)
        direct_includes = [(delimiter, header) for delimiter, header in model.includes if Path(header).name.lower() not in HARNESS_HEADERS and "test/cpp" not in header.replace("\\", "/").lower()]
        required_headers = list(dict.fromkeys(header for _, header in direct_includes))
        if self.shared_header and any(Path(header).name.lower() in HARNESS_HEADERS for _, header in model.includes):
            required_headers.append(self.shared_header)
        for item in self.external_metadata:
            if item.source and normalized_path(item.source) in normalized_path(self.path.as_posix()):
                required_headers.extend(item.required_headers)
                required_headers.extend(registration.header for registration in item.registrations if registration.header)
        for candidate in candidates:
            if candidate.metadata:
                required_headers.extend(candidate.metadata.required_headers)
                required_headers.extend(registration.header for registration in candidate.metadata.registrations if registration.header)
        required_headers = list(dict.fromkeys(required_headers))
        shared_types = shared_test_type_names(self.shared_header, tu)
        return ParsedSource(
            model=model,
            translation_unit=tu,
            candidates=candidates,
            declarations=preserved_declarations(tu, model, candidates, shared_types),
            direct_includes=direct_includes,
            required_headers=required_headers,
            macro_tests=macro_tests(model),
            diagnostics=diagnostics,
            global_reset_expressions=global_reset_expressions,
            global_type_names=source_global_type_names(tu, model, shared_types),
            global_enum_constants=source_global_enum_constants(tu, model),
        )


def nearest_semicolon(tokens: Sequence[Any], end: int, model: SourceModel) -> int | None:
    for token in tokens:
        if token.spelling == ";" and cursor_offset(token.extent.start, model) >= end:
            return cursor_offset(token.extent.end, model)
    return None


def registration_is_file_visible(item: Registration, global_type_names: set[str]) -> bool:
    if item.kind in {"native", "raw_world"}:
        return registration_type_is_global(item.type_name, global_type_names)
    return True


def candidate_uses_registration_helper(candidate: Candidate, global_type_names: set[str]) -> bool:
    return bool(
        (candidate.metadata and candidate.metadata.registration_hooks)
        or any(registration_is_file_visible(item, global_type_names) for item in candidate.registrations)
    )


def inject_registration(candidate: Candidate, model: SourceModel, body_text: str, registration_function_name: str, global_type_names: set[str]) -> tuple[str, list[str]]:
    needs = candidate_uses_registration_helper(candidate, global_type_names)
    if not needs or not candidate.world_variables:
        return body_text, []
    tokens = list(candidate.translation_unit.get_tokens(extent=candidate.body_cursor.extent))
    insertions: list[tuple[int, str]] = []
    for child in walk(candidate.cursor):
        if kind(child) != "VAR_DECL" or not same_source(child, model.path):
            continue
        start, end = cursor_range(child, model)
        if start < candidate.body_range[0] or end > candidate.body_range[1] or getattr(child, "spelling", "") not in candidate.world_variables:
            continue
        semicolon = nearest_semicolon(tokens, end, model)
        if semicolon is not None:
            insertions.append((semicolon - candidate.body_range[0], f"\n\t{registration_function_name}({child.spelling});"))
    if not insertions:
        return body_text, ["required_registration_injection_range_not_found"]
    for offset, value in sorted(set(insertions), reverse=True):
        body_text = body_text[:offset] + value + body_text[offset:]
    return body_text, []


def cpp_string(value: str) -> str:
    return value.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n")


def class_name(source: str, case_id: str) -> str:
    digest = hashlib.sha1(f"{source}:{case_id}".encode()).hexdigest()[:10]
    return f"FFlecsGenerated_{sanitize_identifier(case_id)}_{digest}Test"


def source_support_name(prefix: str, source: str) -> str:
    return f"{prefix}_{hashlib.sha1(source.encode()).hexdigest()[:10]}"


def registration_function(
    registrations: Sequence[Registration],
    hooks: Sequence[str],
    name: str,
    type_aliases: set[str] | None = None,
) -> list[str]:
    if not registrations and not hooks:
        return []
    lines = [f"static void {name}(flecs::world& World)", "{", "\t// Explicit registrations; no automatic registration is assumed."]
    emitted_hooks: set[str] = set()
    type_aliases = type_aliases or set()
    for item in registrations:
        if item.kind in {"native", "raw_world"}:
            # Check each component of a qualified name.  A source-local alias
            # can rename the owning namespace/type token (for example
            # ``Parent::Child``), so matching only the complete qualified
            # spelling would miss the alias and lose the explicit upstream
            # registration name.
            identifiers = set(re.findall(r"[A-Za-z_]\w*", item.type_name))
            if identifiers & type_aliases:
                lines.append(f'\tWorld.component<{item.type_name}>("{cpp_string(item.type_name)}");')
            else:
                lines.append(f"\tWorld.component<{item.type_name}>();")
        elif item.hook:
            lines.append(f"\t{item.hook}(World);")
            emitted_hooks.add(item.hook)
    for hook in hooks:
        if hook not in emitted_hooks:
            lines.append(f"\t{hook}(World);")
            emitted_hooks.add(hook)
    lines.extend(["}", ""])
    return lines


def fork_cpp_api_rewrites(candidate: Candidate, model: SourceModel) -> list[tuple[int, int, str, int]]:
    """Map fork-renamed C++ members without changing raw ecs_* C API calls."""
    tokens = list(candidate.translation_unit.get_tokens(extent=candidate.body_cursor.extent))
    rewrites: list[tuple[int, int, str, int]] = []
    for index, token in enumerate(tokens):
        start = cursor_offset(token.extent.start, model)
        end = cursor_offset(token.extent.end, model)
        original_spelling = model.text[start:end]
        replacement = {"ensure": "obtain", "ensure_second": "obtain_second"}.get(original_spelling)
        if replacement is None or index == 0 or index + 1 >= len(tokens):
            continue
        if tokens[index - 1].spelling not in {".", "->"} or tokens[index + 1].spelling not in {"(", "<"}:
            continue
        if candidate.body_range[0] <= start < end <= candidate.body_range[1]:
            rewrites.append((start, end, replacement, int(getattr(token.extent.start, "line", 0))))
    return rewrites


def fork_cpp_api_rewrite_lines(candidate: Candidate, model: SourceModel) -> set[int]:
    return {line for _, _, _, line in fork_cpp_api_rewrites(candidate, model) if line}


def apply_fork_cpp_api_rewrites(candidate: Candidate, model: SourceModel, body_text: str) -> str:
    rewrites = fork_cpp_api_rewrites(candidate, model)
    for start, end, replacement, _ in reversed(rewrites):
        relative_start = start - candidate.body_range[0]
        relative_end = end - candidate.body_range[0]
        body_text = body_text[:relative_start] + replacement + body_text[relative_end:]
    return body_text


def apply_harness_rewrites(body_text: str) -> str:
    # `install_test_abort` belongs to the upstream process-wide test harness.
    # Converted death tests are reported unsupported, while supported tests
    # must not retain a call to a helper that is not part of the Unreal test
    # module.
    body_text = re.sub(r"(?m)^[ \t]*install_test_abort\(\);[ \t]*(?:\r?\n|$)", "", body_text)
    # MSVC diagnoses comparisons between Flecs' integral id_t and bool as an
    # unsafe mixed-type operation (C4805). These upstream assertions rely on
    # the ordinary bool-to-integer conversion, so spell the equivalent values
    # explicitly in generated bodies.
    body_text = re.sub(
        r"(?P<value>\b[A-Za-z_]\w*)\s*(?P<operator>==|!=)\s*(?P<boolean>true|false)\b",
        lambda match: f"{match.group('value')} {match.group('operator')} {'1' if match.group('boolean') == 'true' else '0'}",
        body_text,
    )
    return body_text


def render_case(candidate: Candidate, parsed: ParsedSource, project_root: Path, source_display: str, registration_function_name: str, reset_function_name: str) -> list[str]:
    source_text = parsed.model.text[candidate.body_range[0]:candidate.body_range[1]]
    source_text = normalize_newlines(apply_fork_cpp_api_rewrites(candidate, parsed.model, source_text))
    source_text = apply_harness_rewrites(source_text)
    if not candidate.reasons and not candidate.skipped:
        source_text, reasons = inject_registration(candidate, parsed.model, source_text, registration_function_name, parsed.global_type_names)
        candidate.reasons.extend(reasons)
        candidate.reasons = list(dict.fromkeys(candidate.reasons))
    status = candidate.status
    class_id = class_name(source_display, candidate.case_id)
    automation = f"FlecsLibrary.Generated.Macro.{candidate.case_id}" if candidate.macro_generated else f"FlecsLibrary.Generated.{candidate.category}.{candidate.case_id}"
    body_id = f"FlecsGeneratedBody_{sanitize_identifier(candidate.case_id)}_{hashlib.sha1(candidate.case_id.encode()).hexdigest()[:8]}"
    parameter = f"(flecs::world& {candidate.world_parameter})" if candidate.world_parameter and candidate.world_parameter != "__invalid__" else "()"
    lines: list[str] = []
    if status == "converted":
        if candidate.namespace and "(" not in candidate.namespace:
            lines.extend([f"namespace {candidate.namespace}", "{", f"static void {body_id}{parameter} {source_text}", "}", ""])
        else:
            lines.extend([f"static void {body_id}{parameter} {source_text}", ""])
    lines.extend([
        f"IMPLEMENT_SIMPLE_AUTOMATION_TEST({class_id}, \"{cpp_string(automation)}\", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)",
        "",
        f"bool {class_id}::RunTest(const FString&)",
        "{",
        f"\tFlecsGeneratedTest::Begin(TEXT(\"{cpp_string(source_display)}\"), TEXT(\"{cpp_string(candidate.case_id)}\"));",
        "\tflecs::world GeneratedWorld;",
    ])
    if status == "unsupported":
        lines.append(f"\tFlecsGeneratedTest::Unsupported(TEXT(\"{cpp_string('; '.join(candidate.reasons))}\"), {candidate.location.line});")
    elif status == "skipped":
        lines.append(f"\tFlecsGeneratedTest::Skip(TEXT(\"{cpp_string(candidate.metadata.skip_reason if candidate.metadata else 'skipped')}\"));")
    else:
        if candidate_uses_registration_helper(candidate, parsed.global_type_names):
            lines.append(f"\t{registration_function_name}(GeneratedWorld);")
        if candidate.setup_hook:
            lines.append(f"\t{candidate.setup_hook}();")
        reset_hooks = list(dict.fromkeys(
            ([reset_function_name] if parsed.global_reset_expressions else [])
            + ([candidate.reset_hook] if candidate.reset_hook else [])
        ))
        for hook in reset_hooks:
            lines.append(f"\t{hook}();")
        body_call = f"{candidate.namespace}::{body_id}" if candidate.namespace and "(" not in candidate.namespace else body_id
        if candidate.world_parameter:
            lines.append(f"\t{body_call}(GeneratedWorld);")
        else:
            lines.append(f"\t{body_call}();")
        if candidate.teardown_hook:
            lines.append(f"\t{candidate.teardown_hook}();")
        for hook in reversed(reset_hooks):
            lines.append(f"\t{hook}();")
    lines.extend(["\treturn FlecsGeneratedTest::End();", "}", ""])
    return lines


def render_macro_stub(case_id: str, macro: str, location: Location, source_display: str) -> list[str]:
    class_id = class_name(source_display, case_id)
    automation = f"FlecsLibrary.Generated.Macro.{case_id}"
    return [
        f"IMPLEMENT_SIMPLE_AUTOMATION_TEST({class_id}, \"{cpp_string(automation)}\", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)",
        "",
        f"bool {class_id}::RunTest(const FString&)",
        "{",
        f"\tFlecsGeneratedTest::Begin(TEXT(\"{cpp_string(source_display)}\"), TEXT(\"{cpp_string(case_id)}\"));",
        f"\tFlecsGeneratedTest::Unsupported(TEXT(\"macro_generated_test_requires_compiler_expansion:{cpp_string(macro)}\"), {location.line});",
        "\treturn FlecsGeneratedTest::End();",
        "}",
        "",
    ]


def render_source(parsed: ParsedSource, project_root: Path, shared_header: str | None) -> str:
    source_display = display_path(parsed.model.path, project_root) or parsed.model.path.name
    registration_function_name = source_support_name("RegisterFlecsGeneratedTestTypes", source_display)
    reset_function_name = source_support_name("ResetFlecsGeneratedFileState", source_display)
    lines = [GENERATED_HEADER, f"// Source: {source_display}", "// Generated by the libclang Flecs converter.", "", "#if WITH_AUTOMATION_TESTS", "", '#include "Misc/AutomationTest.h"']
    seen: set[str] = {"Misc/AutomationTest.h"}
    for delimiter, header in parsed.direct_includes:
        if header not in seen:
            lines.append(f"#include {delimiter}{header}{'>' if delimiter == '<' else chr(34)}")
            seen.add(header)
    for header in parsed.required_headers:
        if header not in seen:
            lines.append(f'#include "{header}"')
            seen.add(header)
    if "flecs.h" not in seen:
        lines.append('#include "flecs.h"')
    lines.append('#include "Bake/FlecsGeneratedTestUtils.h"')
    lines.append("")
    renamed_aliases = {
        name: replacement
        for name, replacement in UE_TYPE_ALIASES.items()
        if name in parsed.global_type_names
    }
    for name, replacement in sorted(renamed_aliases.items()):
        lines.append(f"#define {name} {replacement}")
    shared_type_names = shared_test_type_names(shared_header)
    type_aliases = {
        name: f"{source_support_name('FlecsGeneratedType', source_display)}_{sanitize_identifier(name)}"
        for name in sorted(parsed.global_type_names - shared_type_names - set(UE_TYPE_ALIASES) - parsed.global_enum_constants)
        if "::" not in name
    }
    for name, replacement in type_aliases.items():
        lines.append(f"#define {name} {replacement}")
    enum_aliases = {
        name: f"{source_support_name('FlecsGeneratedEnum', source_display)}_{sanitize_identifier(name)}"
        for name in sorted(parsed.global_enum_constants)
    }
    for name, replacement in enum_aliases.items():
        lines.append(f"#define {name} {replacement}")
    if renamed_aliases or type_aliases or enum_aliases:
        lines.append("")
    for macro in parsed.model.macros:
        match = re.match(r"\s*#define\s+([A-Za-z_]\w*)", macro)
        if match and match.group(1) not in PRESERVED_MACRO_EXCLUSIONS:
            lines.extend([normalize_newlines(macro), ""])
    if parsed.declarations and any(candidate.status == "converted" for candidate in parsed.candidates):
        lines.extend([normalize_newlines(parsed.declarations), ""])
    if parsed.global_reset_expressions and any(candidate.status == "converted" for candidate in parsed.candidates):
        lines.extend([f"static void {reset_function_name}()", "{"])
        lines.extend(f"\t{expression} = {{}};" for expression in parsed.global_reset_expressions)
        lines.extend(["}", ""])
    registrations: list[Registration] = []
    hooks: list[str] = []
    for candidate in parsed.candidates:
        if candidate.status != "converted":
            continue
        for item in candidate.registrations:
            if not registration_is_file_visible(item, parsed.global_type_names):
                continue
            if item not in registrations:
                registrations.append(item)
            if item.hook and item.hook not in hooks:
                hooks.append(item.hook)
        if candidate.metadata:
            for hook in candidate.metadata.registration_hooks:
                if hook not in hooks:
                    hooks.append(hook)
    lines.extend(registration_function(registrations, hooks, registration_function_name, set(type_aliases)))
    for candidate in parsed.candidates:
        lines.extend(render_case(candidate, parsed, project_root, source_display, registration_function_name, reset_function_name))
    for case_id, macro, location in parsed.macro_tests:
        if not any(candidate.location.line == location.line for candidate in parsed.candidates):
            lines.extend(render_macro_stub(case_id, macro, location, source_display))
    if renamed_aliases or type_aliases or enum_aliases:
        lines.append("")
        for name in sorted(renamed_aliases):
            lines.append(f"#undef {name}")
        for name in sorted(type_aliases):
            lines.append(f"#undef {name}")
        for name in sorted(enum_aliases):
            lines.append(f"#undef {name}")
    lines.extend(["#endif // WITH_AUTOMATION_TESTS", ""])
    return "\n".join(lines)


def candidate_entry(candidate: Candidate, parsed: ParsedSource, project_root: Path, generated: str, digest: str) -> dict[str, Any]:
    automation = f"FlecsLibrary.Generated.Macro.{candidate.case_id}" if candidate.macro_generated else f"FlecsLibrary.Generated.{candidate.category}.{candidate.case_id}"
    return {
        "source": display_path(candidate.source, project_root),
        "case": candidate.case_id,
        "function": candidate.name,
        "generated": generated,
        "automation": automation,
        "status": candidate.status,
        "reason": "; ".join(candidate.reasons) if candidate.reasons else candidate.metadata.skip_reason if candidate.metadata and candidate.metadata.skip_reason else None,
        "fixture": candidate.metadata.fixture if candidate.metadata else "raw_world",
        "category": candidate.category,
        "tags": candidate.tags,
        "source_location": candidate.location.json(),
        "conditions": candidate.conditions,
        "required_headers": parsed.required_headers + (candidate.metadata.required_headers if candidate.metadata else []),
        "required_registrations": [item.json() for item in candidate.registrations],
        "registration_hooks": candidate.metadata.registration_hooks if candidate.metadata else [],
        "assertions": [item.json() for item in candidate.assertions],
        "source_hash": digest,
        "macro_generated": candidate.macro_generated,
        "order": candidate.location.line,
    }


class Converter:
    def __init__(self, project_root: Path, upstream: Path, fork: Path | None, output: Path, metadata: Path | None, inventory: Path | None, libclang: Path | None, include_dirs: Sequence[Path], clang_args: Sequence[str], shared_header: str | None) -> None:
        self.project_root = project_root.resolve()
        self.upstream = upstream.resolve()
        self.fork = fork.resolve() if fork else None
        self.output = output.resolve()
        self.metadata_path = metadata.resolve() if metadata else None
        self.inventory_path = inventory.resolve() if inventory else None
        self.metadata = load_metadata(self.metadata_path)
        self.inventory = load_inventory(self.inventory_path)
        self.include_dirs = [(item if item.is_absolute() else self.project_root / item).resolve() for item in include_dirs]
        self.clang_args = list(clang_args)
        self.shared_header = shared_header
        self.frontend = Frontend(libclang)

    def clang_arguments(self) -> list[str]:
        args = [
            "-x",
            "c++",
            "-std=c++20",
            "-fparse-all-comments",
            "-Wno-everything",
            "-DUE_BUILD_DEBUG=0",
            "-DUE_BUILD_DEVELOPMENT=1",
            "-DUE_BUILD_TEST=0",
            "-DUE_BUILD_SHIPPING=0",
            "-DWITH_EDITOR=1",
            "-DWITH_EDITORONLY_DATA=1",
            "-DWITH_ENGINE=1",
            "-DWITH_UNREAL_DEVELOPER_TOOLS=1",
            "-DWITH_PLUGIN_SUPPORT=1",
            "-DIS_MONOLITHIC=0",
            "-DIS_PROGRAM=0",
            "-DUBT_COMPILED_PLATFORM=Windows",
            "-DPLATFORM_WINDOWS=1",
            "-DPLATFORM_64BITS=1",
            "-DFLECSLIBRARY_API=",
            "-DFLECS_TEST_CONVERTER",
            "-DFLECS_CPP",
            "-DFLECS_MODULE",
            "-DFLECS_SCRIPT",
            "-DFLECS_PARSER",
            "-DFLECS_QUERY_DSL",
            "-DFLECS_SYSTEM",
            "-DFLECS_PIPELINE",
            "-DFLECS_TIMER",
            "-DFLECS_META",
            "-DFLECS_JSON",
            "-DFLECS_SCRIPT_MATH",
        ]
        source = self.project_root / "Plugins" / "Unreal-Flecs" / "Source" / "FlecsLibrary"
        compatibility = Path(__file__).resolve().parent / "FlecsTestConversionIncludes"
        candidates = [
            compatibility,
            *engine_include_roots(self.project_root),
            self.project_root,
            self.upstream,
            self.upstream.parent / "include",
            *self.include_dirs,
            source / "Public",
            source / "Private",
            source / "Tests",
            self.project_root / "Plugins" / "Unreal-Flecs" / "Source" / "SolidMacros" / "Public",
        ]
        seen: set[Path] = set()
        for include in candidates:
            if not include.exists():
                continue
            resolved = include.resolve()
            if resolved in seen:
                continue
            seen.add(resolved)
            # Keep the option and its value distinct: libclang receives an argv
            # array, and a Windows path with spaces is not reliably handled as
            # one combined -I<path> argument.
            args.extend(["-I", str(resolved)])
        args.extend(self.clang_args)
        return args

    @staticmethod
    def sources(root: Path | None) -> list[Path]:
        if root is None or not root.exists():
            return []
        return sorted(
            path
            for path in root.rglob("*")
            if path.is_file()
            and path.name.lower() != "main.cpp"
            and path.suffix.lower() in {".c", ".cc", ".cpp", ".cxx"}
            and "generated" not in {part.lower() for part in path.parts}
        )

    def generated_path(self, root: Path, source: Path, origin: str) -> Path:
        try:
            relative = source.resolve().relative_to(root.resolve())
        except ValueError:
            relative = Path(source.name)
        stem = sanitize_identifier("_".join(relative.with_suffix("").parts))
        path_hash = hashlib.sha1(relative.as_posix().encode("utf-8")).hexdigest()[:8]
        return self.output / f"{origin}_{stem}_{path_hash}.generated.cpp"

    def remove_stale_outputs(self, current: set[Path]) -> None:
        for path in self.output.glob("*.generated.cpp"):
            if path.resolve() in current:
                continue
            try:
                header = path.read_text(encoding="utf-8", errors="replace")[:4096]
            except OSError:
                continue
            if GENERATED_HEADER in header:
                path.unlink()

    @staticmethod
    def generated_inventory(manifest: dict[str, Any]) -> dict[str, Any]:
        tests = []
        for item in manifest["tests"]:
            case_id = item.get("case")
            if not isinstance(case_id, str) or case_id.startswith("<"):
                continue
            category = item.get("category") or "Flecs"
            automation = item.get("automation") or f"FlecsLibrary.Generated.{category}.{case_id}"
            tests.append({
                "source": item.get("source"),
                "case": case_id,
                "automation": automation,
                "generated": item.get("generated"),
                "status": item.get("status"),
            })
        return {
            "schema_version": SCHEMA_VERSION,
            "category": "FlecsLibrary.Generated",
            "tests": tests,
            "summary": manifest["summary"],
        }

    def convert(self) -> dict[str, Any]:
        if not self.upstream.exists():
            raise ConverterError(f"Upstream test root does not exist: {self.upstream}")
        if self.fork and not self.fork.exists():
            raise ConverterError(f"Fork test root does not exist: {self.fork}")
        for input_root in [self.upstream, self.fork] if self.fork else [self.upstream]:
            try:
                self.output.relative_to(input_root)
            except ValueError:
                continue
            raise ConverterError(f"Generated output must not be inside an input root: {self.output}")
        self.output.mkdir(parents=True, exist_ok=True)
        roots = [self.upstream] + ([self.fork] if self.fork else [])
        source_list = [("Upstream", self.upstream, path) for path in self.sources(self.upstream)]
        if self.fork:
            source_list.extend(("Fork", self.fork, path) for path in self.sources(self.fork))
        if not source_list:
            raise ConverterError("No C++ test sources were found in the configured input roots")
        entries: list[dict[str, Any]] = []
        generated_files: set[str] = set()
        generated_paths: set[Path] = set()
        seen_ids: set[str] = set()
        known_sources: set[str] = set()
        diagnostics: list[str] = []
        for origin, root, source in source_list:
            source_display = display_path(source, self.project_root) or source.name
            known_sources.add(source_display)
            digest = sha256_file(source)
            try:
                parsed = SourceParser(self.frontend, source, self.clang_arguments(), self.metadata, self.inventory, roots, self.shared_header).parse()
            except Exception as error:
                entries.append({"source": source_display, "case": "<source>", "function": None, "generated": None, "status": "unsupported", "reason": f"parse_error:{error}", "fixture": "raw_world", "category": None, "tags": [], "source_location": {"line": 1, "column": 1}, "conditions": [], "required_headers": [], "required_registrations": [], "registration_hooks": [], "assertions": [], "source_hash": digest, "macro_generated": False, "order": 0})
                diagnostics.append(f"{source_display}: {error}")
                continue
            if not parsed.candidates and not parsed.macro_tests:
                entries.append({"source": source_display, "case": "<no-test-functions>", "function": None, "generated": None, "status": "skipped", "reason": "no recognized Flecs test functions", "fixture": "raw_world", "category": None, "tags": [], "source_location": None, "conditions": [], "required_headers": parsed.required_headers, "required_registrations": [], "registration_hooks": [], "assertions": [], "source_hash": digest, "macro_generated": False, "order": 0})
                continue
            output_path = self.generated_path(root, source, origin)
            output_display = display_path(output_path, self.project_root) or output_path.name
            generated_files.add(output_display)
            generated_paths.add(output_path.resolve())
            for candidate in parsed.candidates:
                if candidate.case_id in seen_ids:
                    candidate.reasons.append("duplicate_test_id")
                seen_ids.add(candidate.case_id)
            output_path.write_text(render_source(parsed, self.project_root, self.shared_header), encoding="utf-8", newline="\n")
            for candidate in parsed.candidates:
                entries.append(candidate_entry(candidate, parsed, self.project_root, output_display, digest))
            for case_id, macro, location in parsed.macro_tests:
                if any(candidate.location.line == location.line for candidate in parsed.candidates):
                    continue
                entries.append({"source": source_display, "case": case_id, "function": None, "generated": output_display, "automation": f"FlecsLibrary.Generated.Macro.{case_id}", "status": "unsupported", "reason": "macro_generated_test_requires_compiler_expansion", "fixture": "raw_world", "category": "Macro", "tags": [], "source_location": location.json(), "conditions": parsed.model.conditions.get(location.line, []), "required_headers": parsed.required_headers, "required_registrations": [], "registration_hooks": [], "assertions": [], "source_hash": digest, "macro_generated": True, "macro": macro, "order": location.line})
            diagnostics.extend(f"{source_display}: {item}" for item in parsed.diagnostics if "error" in item.lower())
        represented = {entry.get("case") for entry in entries if entry.get("source") in known_sources}
        for expected in self.inventory:
            if expected.case_id not in represented:
                entries.append({"source": expected.source, "case": expected.case_id, "function": None, "generated": None, "automation": f"FlecsLibrary.Generated.{expected.category or 'Flecs'}.{expected.case_id}", "status": "skipped", "reason": "upstream_inventory_case_not_found", "fixture": "raw_world", "category": expected.category, "tags": list(expected.tags), "source_location": None, "conditions": [], "required_headers": [], "required_registrations": [], "registration_hooks": [], "assertions": [], "source_hash": None, "macro_generated": False, "order": 0})
        entries.sort(key=lambda item: (str(item.get("source")), int(item.get("order", 0)), str(item.get("case"))))
        summary = {status: sum(1 for item in entries if item.get("status") == status) for status in ("converted", "skipped", "unsupported")}
        manifest = {"schema_version": SCHEMA_VERSION, "backend": "raw_world", "generated_file_header": GENERATED_HEADER, "source_roots": {"upstream": display_path(self.upstream, self.project_root), "fork": display_path(self.fork, self.project_root)}, "generated_root": display_path(self.output, self.project_root), "inventory": display_path(self.inventory_path, self.project_root), "metadata": display_path(self.metadata_path, self.project_root), "tests": entries, "generated_files": sorted(generated_files), "summary": summary, "diagnostics": diagnostics}
        generated_inventory_path = self.output / "flecs_generated_test_inventory.json"
        generated_inventory_path.write_text(json.dumps(self.generated_inventory(manifest), indent=2) + "\n", encoding="utf-8", newline="\n")
        manifest["generated_inventory"] = display_path(generated_inventory_path, self.project_root)
        self.remove_stale_outputs(generated_paths)
        (self.output / "flecs_conversion_manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8", newline="\n")
        return manifest


def validate_manifest(path: Path, project_root: Path) -> list[str]:
    try:
        manifest = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        return [f"manifest_read_error:{error}"]
    if not isinstance(manifest, dict):
        return ["manifest_is_not_object"]
    errors: list[str] = []
    if manifest.get("schema_version") != SCHEMA_VERSION:
        errors.append("manifest_schema_version_mismatch")
    tests = manifest.get("tests")
    if not isinstance(tests, list):
        return errors + ["manifest_tests_is_not_array"]
    allowed = {"converted", "skipped", "unsupported"}
    counts = {status: 0 for status in allowed}
    seen: set[tuple[Any, Any]] = set()
    seen_cases: set[Any] = set()
    for item in tests:
        if not isinstance(item, dict):
            errors.append("manifest_entry_is_not_object")
            continue
        key = (item.get("source"), item.get("case"))
        if key in seen:
            errors.append(f"duplicate_manifest_test:{key}")
        seen.add(key)
        case_id = item.get("case")
        if isinstance(case_id, str) and not case_id.startswith("<"):
            if case_id in seen_cases:
                errors.append(f"duplicate_manifest_test_id:{case_id}")
            seen_cases.add(case_id)
        status = item.get("status")
        if status not in allowed:
            errors.append(f"invalid_status:{key}")
            continue
        counts[status] += 1
        if status in {"unsupported", "skipped"} and not item.get("reason"):
            errors.append(f"missing_reason:{key}")
        if item.get("generated"):
            generated = resolve_display_path(str(item["generated"]), project_root)
            if not generated.is_file():
                errors.append(f"missing_generated_file:{item['generated']}")
            else:
                text = generated.read_text(encoding="utf-8", errors="replace")
                if GENERATED_HEADER not in text:
                    errors.append(f"missing_generated_header:{item['generated']}")
                if status == "converted" and "IMPLEMENT_SIMPLE_AUTOMATION_TEST" not in text:
                    errors.append(f"converted_without_automation_test:{key}")
                if status == "unsupported" and "FlecsGeneratedTest::Unsupported" not in text:
                    errors.append(f"unsupported_without_diagnostic:{key}")
        source = item.get("source")
        expected_hash = item.get("source_hash")
        if source and expected_hash:
            source_path = resolve_display_path(str(source), project_root)
            if not source_path.is_file():
                errors.append(f"missing_source_file:{source}")
            elif sha256_file(source_path) != expected_hash:
                errors.append(f"source_changed_since_generation:{source}")
    if manifest.get("summary") != counts:
        errors.append("manifest_summary_mismatch")
    generated_inventory = manifest.get("generated_inventory")
    if generated_inventory:
        inventory_path = resolve_display_path(str(generated_inventory), project_root)
        try:
            inventory = json.loads(inventory_path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as error:
            errors.append(f"generated_inventory_read_error:{error}")
        else:
            if not isinstance(inventory, dict):
                errors.append("generated_inventory_is_not_object")
                return errors
            if inventory.get("category") != "FlecsLibrary.Generated":
                errors.append("generated_inventory_category_mismatch")
            if inventory.get("summary") != manifest.get("summary"):
                errors.append("generated_inventory_summary_mismatch")
            manifest_ids = {(item.get("source"), item.get("case")) for item in tests if isinstance(item.get("case"), str) and not item["case"].startswith("<")}
            inventory_tests = inventory.get("tests", [])
            if not isinstance(inventory_tests, list):
                errors.append("generated_inventory_tests_is_not_array")
                inventory_tests = []
            inventory_ids = {(item.get("source"), item.get("case")) for item in inventory_tests if isinstance(item, dict)}
            if manifest_ids != inventory_ids:
                errors.append("generated_inventory_test_set_mismatch")
    return errors


def default_upstream(project_root: Path) -> Path:
    choices = [project_root / "Plugins" / "Unreal-Flecs" / "Scripts" / "Upstream", project_root / "test" / "cpp" / "src", project_root / "test", project_root / "Tests" / "Upstream"]
    return next((choice for choice in choices if choice.exists()), choices[0])


def default_fork(project_root: Path) -> Path | None:
    choice = project_root / "Plugins" / "Unreal-Flecs" / "Scripts" / "ForkTests"
    return choice if choice.exists() else None


def resolve_from_root(path: Path | None, project_root: Path) -> Path | None:
    if path is None:
        return None
    return (path if path.is_absolute() else project_root / path).resolve()


def argument_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("read_directory", nargs="?", type=Path)
    parser.add_argument("write_directory", nargs="?", type=Path)
    parser.add_argument("--project-root", type=Path)
    parser.add_argument("--upstream-root", type=Path)
    parser.add_argument("--fork-root", type=Path)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--metadata", type=Path)
    parser.add_argument("--inventory", type=Path)
    parser.add_argument("--libclang", type=Path)
    parser.add_argument("--include-dir", type=Path, action="append", default=[])
    parser.add_argument("--clang-arg", action="append", default=[])
    parser.add_argument("--shared-test-header", default="Bake/FlecsTestTypes.h")
    parser.add_argument("--no-shared-test-header", action="store_true")
    parser.add_argument("--validate", action="store_true")
    parser.add_argument("--verbose", action="store_true")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    options = argument_parser().parse_args(argv)
    logging.basicConfig(level=logging.DEBUG if options.verbose else logging.INFO, format="%(levelname)s: %(message)s")
    project_root = (options.project_root or Path(__file__).resolve().parents[3]).resolve()
    upstream = resolve_from_root(options.upstream_root or options.read_directory or default_upstream(project_root), project_root)
    output = resolve_from_root(options.output or options.write_directory or project_root / "Plugins" / "Unreal-Flecs" / "Source" / "FlecsLibrary" / "Tests" / "Generated", project_root)
    fork = resolve_from_root(options.fork_root, project_root) or default_fork(project_root)
    metadata = resolve_from_root(options.metadata, project_root)
    inventory = resolve_from_root(options.inventory, project_root)
    libclang = resolve_from_root(options.libclang, project_root)
    try:
        manifest = Converter(project_root, upstream, fork, output, metadata, inventory, libclang, options.include_dir, options.clang_arg, None if options.no_shared_test_header else options.shared_test_header).convert()
        if options.validate:
            errors = validate_manifest(output / "flecs_conversion_manifest.json", project_root)
            if errors:
                for error in errors:
                    LOG.error(error)
                return 2
        LOG.info("Converted %d, skipped %d, unsupported %d", manifest["summary"]["converted"], manifest["summary"]["skipped"], manifest["summary"]["unsupported"])
        return 0
    except ConverterError as error:
        LOG.error("%s", error)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
