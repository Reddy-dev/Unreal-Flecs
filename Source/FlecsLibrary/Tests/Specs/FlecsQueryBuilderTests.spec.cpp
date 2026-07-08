// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "Bake/FlecsTestUtils.h"

// @TODO: add tests for query builder

#if WITH_AUTOMATION_TESTS && defined(FLECS_TESTS)

#include "flecs.h"

#include "Bake/FlecsTestTypes.h"

BEGIN_DEFINE_SPEC(FFlecsQueryBuilderTestsSpec,
                  "FlecsLibrary.FlecsQueryBuilderTests",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

END_DEFINE_SPEC(FFlecsQueryBuilderTestsSpec);

void FFlecsQueryBuilderTestsSpec::Define()
{
	
}

#endif // WITH_AUTOMATION_TESTS
