// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Worlds/FlecsStage.h"
#include "Worlds/FlecsWorld.h"

/**
 * Layout of the tests:
 * A. Stage Object Tests Through System
 */
TEST_CLASS_WITH_FLAGS_AND_TAGS(A12_UnrealFlecsStageTests,
                               "UnrealFlecs.A12_FlecsStageTests",
                               EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
                               | EAutomationTestFlags::CriticalPriority,
                               "[Flecs][Stage]")
{
	inline static TUniquePtr<FFlecsTestFixtureRAII> Fixture;
	inline static TObjectPtr<UFlecsWorld> FlecsWorld = nullptr;

	BEFORE_EACH()
	{
		Fixture = TUniquePtr<FFlecsTestFixtureRAII>(new FFlecsTestFixtureRAII({}, {}, EWorldType::Game));
		FlecsWorld = Fixture->Fixture.GetFlecsWorld();
	}

	AFTER_EACH()
	{
		FlecsWorld = nullptr;
		Fixture.Reset();
	}

	TEST_METHOD(A1_MultiThreadedSystemGetStage_IterAPI_ReturnsValidStageObject)
	{
		if (FlecsWorld->GetStageCount() < 2)
		{
			return;
		}

		bool bSystemFired = false;
		UFlecsStage* CapturedStage = nullptr;

		flecs::system TestSystem = FlecsWorld->GetNativeFlecsWorld()
			.system<>()
			.kind(flecs::OnUpdate)
			.multi_threaded()
			.run([&bSystemFired, &CapturedStage](flecs::iter& Iter)
			{
				while (Iter.next())
				{
					bSystemFired = true;
					CapturedStage = FlecsWorld->GetStage(Iter);
				}
			});

		ASSERT_THAT(IsTrue(TestSystem.is_valid()));

		Fixture->Fixture.TickWorld();

		ASSERT_THAT(IsTrue(bSystemFired));
		ASSERT_THAT(IsTrue(CapturedStage != nullptr));
		ASSERT_THAT(IsTrue(CapturedStage->IsStage()));
		ASSERT_THAT(IsTrue(CapturedStage->GetStageId() > 0));
	}

}; // End of A12_UnrealFlecsStageTests

#endif // WITH_AUTOMATION_TESTS
