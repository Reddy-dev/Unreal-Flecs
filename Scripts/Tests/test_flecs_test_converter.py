from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path


SCRIPT_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(SCRIPT_ROOT))

from flecs_test_converter import (  # noqa: E402
    Candidate,
    GENERATED_HEADER,
    ParsedSource,
    Registration,
    build_model,
    parse_metadata_payload,
    registration_function,
    render_source,
    sanitize_identifier,
    shared_test_type_names,
    validate_manifest,
)


class FlecsTestConverterTests(unittest.TestCase):
    def test_metadata_keeps_explicit_id_category_tags_and_registration(self) -> None:
        metadata = parse_metadata_payload(
            'id=Fork_case category=Fork tags="fork,manual" registrations=Position:native'
        )
        self.assertIsNotNone(metadata)
        self.assertEqual(metadata.case_id, "Fork_case")
        self.assertEqual(metadata.category, "Fork")
        self.assertEqual(metadata.tags, ["fork", "manual"])
        self.assertEqual(metadata.registrations[0].type_name, "Position")

    def test_source_model_tracks_conditions_and_nested_directives(self) -> None:
        model = build_model(Path("fixture.cpp"), "#if WITH_TESTS\nvoid A() {}\n#endif\n")
        self.assertEqual(model.conditions[2], ["#if WITH_TESTS"])
        self.assertEqual(model.conditions[3], [])

    def test_identifier_is_stable_and_valid(self) -> None:
        self.assertEqual(sanitize_identifier("entity/new-case"), "entity_new_case")
        self.assertEqual(sanitize_identifier("123 case"), "_123_case")

    def test_shared_registration_is_emitted_by_compatibility_layer(self) -> None:
        self.assertIn("LifecycleTracker", shared_test_type_names("Bake/FlecsTestTypes.h"))
        rendered = registration_function(
            [Registration("Position"), Registration("LocalType")],
            [],
            "RegisterGeneratedTypes",
            shared_type_names={"Position"},
            register_shared_types=True,
        )
        text = "\n".join(rendered)
        self.assertIn("FlecsGeneratedTest::RegisterSharedTypes(World);", text)
        self.assertNotIn("World.component<Position>();", text)
        self.assertIn("World.component<LocalType>();", text)

    def test_manifest_validation_checks_generated_header_and_reason(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            generated = root / "generated.cpp"
            generated.write_text(GENERATED_HEADER + "\n", encoding="utf-8")
            manifest = root / "manifest.json"
            manifest.write_text(json.dumps({
                "schema_version": 1,
                "summary": {"converted": 0, "skipped": 1, "unsupported": 0},
                "tests": [{
                    "source": "missing.cpp",
                    "case": "Case",
                    "status": "skipped",
                    "reason": "fixture",
                    "generated": "generated.cpp",
                    "source_hash": None,
                }],
            }), encoding="utf-8")
            self.assertEqual(validate_manifest(manifest, root), [])

    def test_renderer_matches_golden_fragments(self) -> None:
        source = "{ test_assert(true); }"
        model = build_model(Path("Fixture.cpp"), source)
        candidate = Candidate(
            name="Basic_case",
            case_id="Basic_case",
            category="Basic",
            tags=[],
            source=Path("Fixture.cpp"),
            location=model.location(0, 2, 1),
            function_range=(0, len(source)),
            body_range=(0, len(source)),
            cursor=None,
            body_cursor=None,
            translation_unit=None,
        )
        parsed = ParsedSource(model, None, [candidate], "", [], [], [], [])
        rendered = render_source(parsed, Path("."), None)
        golden = (Path(__file__).parent / "Golden" / "Basic.generated.golden.cpp").read_text(encoding="utf-8")
        for fragment in golden.splitlines():
            if fragment and not fragment.startswith("#") and not fragment.startswith("static void Register"):
                self.assertIn(fragment, rendered)


if __name__ == "__main__":
    unittest.main()
