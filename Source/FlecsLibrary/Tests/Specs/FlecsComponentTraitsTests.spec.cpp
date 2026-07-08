// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "Bake/FlecsTestUtils.h"

#if WITH_AUTOMATION_TESTS && defined(FLECS_TESTS)

BEGIN_DEFINE_SPEC(FFlecsComponentTraitsTestsSpec,
                  "FlecsLibrary.ComponentTraits",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

END_DEFINE_SPEC(FFlecsComponentTraitsTestsSpec);

void FFlecsComponentTraitsTestsSpec::Define()
{
	
}

#endif // WITH_AUTOMATION_TESTS && defined(FLECS_TESTS)
