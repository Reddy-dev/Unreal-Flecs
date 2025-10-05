
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

#include "Fixtures/FlecsWorldFixture.h"

BEGIN_DEFINE_SPEC(FComponentCollectionTestSpec,
                  "Flecs.Components.Collections",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

FFlecsTestFixture Fixture;

END_DEFINE_SPEC(FComponentCollectionTestSpec);

void FComponentCollectionTestSpec::Define()
{

}

#endif // WITH_AUTOMATION_TESTS
