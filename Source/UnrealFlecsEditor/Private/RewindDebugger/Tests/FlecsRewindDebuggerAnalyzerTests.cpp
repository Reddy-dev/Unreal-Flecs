// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "RewindDebugger/FlecsRewindDebuggerAnalyzer.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Debugging/FlecsRewindDebuggerTag.h"
#include "Misc/AutomationTest.h"

static_assert(std::is_empty_v<FFlecsRewindDebuggerTag>,
	"FFlecsRewindDebuggerTag must remain an empty Flecs tag type.");

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFlecsRewindDebuggerPayloadValidationTest,
	"Flecs.RewindDebugger.PayloadValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFlecsRewindDebuggerPayloadValidationTest::RunTest(
	MAYBE_UNUSED const FString& Parameters)
{
	using UE::Flecs::RewindDebugger::ValidateSnapshotPayload;

	constexpr uint64 EntityId = 4294967396ull;
	const FString ValidPayload = TEXT(
		"{"
			"\"schemaVersion\":1,"
			"\"entityId\":\"4294967396\","
			"\"generation\":1,"
			"\"name\":\"DebugEntity\","
			"\"path\":\"Parent.DebugEntity\","
			"\"members\":["
				"{"
					"\"id\":\"100\","
					"\"name\":\"Position\","
					"\"kind\":\"component\","
					"\"valueAvailable\":true,"
					"\"value\":{\"x\":1}"
				"},"
				"{"
					"\"id\":\"101\","
					"\"name\":\"Unreflected\","
					"\"kind\":\"component\","
					"\"valueAvailable\":false"
				"}"
			"]"
		"}");

	TestTrue(TEXT("A well-formed snapshot is accepted"),
		ValidateSnapshotPayload(ValidPayload, EntityId));
	TestFalse(TEXT("A mismatched complete entity ID is rejected"),
		ValidateSnapshotPayload(ValidPayload, EntityId + 1));
	TestFalse(TEXT("Malformed JSON is rejected"),
		ValidateSnapshotPayload(TEXT("{"), EntityId));
	TestFalse(TEXT("An available value without a value field is rejected"),
		ValidateSnapshotPayload(
			TEXT(
				"{"
					"\"schemaVersion\":1,"
					"\"entityId\":\"4294967396\","
					"\"generation\":1,"
					"\"name\":\"DebugEntity\","
					"\"path\":\"DebugEntity\","
					"\"members\":[{"
						"\"id\":\"100\","
						"\"name\":\"Position\","
						"\"kind\":\"component\","
						"\"valueAvailable\":true"
					"}]"
				"}"),
			EntityId));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
