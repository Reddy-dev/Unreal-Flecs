// Elie Wiese-Namir Â© 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Worlds/FlecsStage.h"
#include "Worlds/FlecsWorld.h"

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(UnrealFlecsStageTests,
								   "UnrealFlecs.World.Stages",
                               EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
                               | EAutomationTestFlags::CriticalPriority,
                               "[Flecs][Stage]")
{
protected:
	virtual EWorldType::Type WorldType() const override
	{
		return EWorldType::Game;
	}

public:

	TEST_METHOD(MultiThreadedSystemGetStage_IterAPI_ReturnsValidStageObject)
	{
		if (World()->GetStageCount() < 2)
		{
			return;
		}

		bool bSystemFired = false;
		UFlecsStage* CapturedStage = nullptr;

		flecs::system TestSystem = World()->GetNativeFlecsWorld()
			.system<>()
			.kind(flecs::OnUpdate)
			.multi_threaded()
			.run([&bSystemFired, &CapturedStage, this](flecs::iter& Iter)
			{
				while (Iter.next())
				{
					bSystemFired = true;
					// idk how legal this is
					CapturedStage = World()->GetStage(Iter);
				}
			});

		ASSERT_THAT(IsTrue(TestSystem.is_valid()));

		TickWorld();

		ASSERT_THAT(IsTrue(bSystemFired));
		ASSERT_THAT(IsTrue(CapturedStage != nullptr));
		ASSERT_THAT(IsTrue(CapturedStage->IsStage()));
		ASSERT_THAT(IsTrue(CapturedStage->GetStageId() > 0));
	}

}; // UnrealFlecsStageTests

#endif // WITH_AUTOMATION_TESTS
