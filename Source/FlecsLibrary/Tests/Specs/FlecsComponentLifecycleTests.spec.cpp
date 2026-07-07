// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "Bake/FlecsTestUtils.h"

#if WITH_AUTOMATION_TESTS && defined(FLECS_TESTS)

#include "flecs.h"

#include "Bake/FlecsTestTypes.h"

BEGIN_DEFINE_SPEC(FFlecsComponentLifecycleTestsSpec,
                  "FlecsLibrary.ComponentLifecycle",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

END_DEFINE_SPEC(FFlecsComponentLifecycleTestsSpec);

void FFlecsComponentLifecycleTestsSpec::Define()
{
	// Test cases are defined here
}

#endif // WITH_AUTOMATION_TESTS && defined(FLECS_TESTS)
