// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

#include "Pipelines/TickFunctions/FlecsTickTypeNativeTags.h"
#include "Systems/FlecsSystem.h"
#include "Worlds/FlecsWorld.h"

#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

/**
 * Layout of the tests:
 * A. World Tick Tests
 */
TEST_CLASS_WITH_FLAGS(A11_UnrealFlecsWorldTickTests,
							   "UnrealFlecs.A11_FlecsWorldTickTests",
							   EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
								| EAutomationTestFlags::CriticalPriority)
{
	inline static TUniquePtr<FFlecsTestFixtureRAII> Fixture;
	inline static TObjectPtr<UFlecsWorld> FlecsWorld = nullptr;

	BEFORE_EACH()
	{
		Fixture = TUniquePtr<FFlecsTestFixtureRAII>(new FFlecsTestFixtureRAII({}, {}, {}, EWorldType::Game));
		FlecsWorld = Fixture->Fixture.GetFlecsWorld();
	}

	AFTER_EACH()
	{
		FlecsWorld = nullptr;
	}
	
	TEST_METHOD(A1_CreateSystemsInMainLoopAndTickWorld)
	{
		static constexpr double TickDeltaTime = 1.0 / 60.0;
		
		int32 Counter = 0;
		FFlecsSystem TestSystem = FlecsWorld->World.system<>()
			.kind(flecs::OnUpdate)
			.each([this, &Counter](flecs::iter& Iter, size_t Index)
			{
				AddErrorIfFalse(FMath::IsNearlyEqual(Iter.delta_time(), TickDeltaTime),
					FString::Printf(TEXT("Expected delta time: %f, but got: %f"),
						TickDeltaTime,
						Iter.delta_time()
					)
				);
				
				Counter++;
			});

		int32 PostUpdateCounter = 0;
		FFlecsSystem TestSystemPostUpdate = FlecsWorld->World.system<>()
			.kind(flecs::PostUpdate)
			.each([this, &PostUpdateCounter](flecs::iter& Iter, size_t Index)
			{
				AddErrorIfFalse(FMath::IsNearlyEqual(Iter.delta_time(), TickDeltaTime),
					FString::Printf(TEXT("Expected delta time: %f, but got: %f"),
						TickDeltaTime,
						Iter.delta_time()
					)
				);
				
				PostUpdateCounter++;
			});
		
		ASSERT_THAT(IsTrue(TestSystem.IsValid()));
		ASSERT_THAT(IsTrue(TestSystemPostUpdate.IsValid()));
		
		ASSERT_THAT(AreEqual(0, Counter));
		ASSERT_THAT(AreEqual(0, PostUpdateCounter));

		Fixture->Fixture.TickWorld(TickDeltaTime);
		
		ASSERT_THAT(AreEqual(1, Counter));
		ASSERT_THAT(AreEqual(1, PostUpdateCounter));

		Fixture->Fixture.TickWorld(TickDeltaTime);
		
		ASSERT_THAT(AreEqual(2, Counter));
		ASSERT_THAT(AreEqual(2, PostUpdateCounter));
	}

	TEST_METHOD(A2_CreateSystemsInUnrealTickTypesAndMainLoopAndTickWorld)
	{
		// @TODO: use FFlecsSystem instead of FFlecsEntityHandle

		int32 MainLoopCounter = 0;
		FFlecsEntityHandle MainLoopSystem = FlecsWorld->World.system<>()
			.kind(flecs::OnUpdate)
			.each([&MainLoopCounter](flecs::iter& Iter, size_t Index)
			{
				MainLoopCounter++;
			});
			//.add(FlecsWorld->GetTagEntity(FlecsTickType_MainLoop).GetFlecsId());
		
		int32 PrePhysicsCounter = 0;
		FFlecsEntityHandle PrePhysicsSystem = FlecsWorld->World.system<>()
			.kind(flecs::OnUpdate)
			.each([&PrePhysicsCounter](flecs::iter& Iter, size_t Index)
			{
				PrePhysicsCounter++;
			})
			.add(FlecsWorld->GetTagEntity(FlecsTickType_PrePhysics).GetFlecsId());

		int32 DuringPhysicsCounter = 0;
		FFlecsEntityHandle DuringPhysicsSystem = FlecsWorld->World.system<>()
			.kind(flecs::OnUpdate)
			.each([&DuringPhysicsCounter](flecs::iter& Iter, size_t Index)
			{
				DuringPhysicsCounter++;
			})
			.add(FlecsWorld->GetTagEntity(FlecsTickType_DuringPhysics).GetFlecsId());

		int32 PostPhysicsCounter = 0;
		FFlecsEntityHandle PostPhysicsSystem = FlecsWorld->World.system<>()
			.kind(flecs::OnUpdate)
			.each([&PostPhysicsCounter](flecs::iter& Iter, size_t Index)
			{
				PostPhysicsCounter++;
			})
			.add(FlecsWorld->GetTagEntity(FlecsTickType_PostPhysics).GetFlecsId());

		int32 PostUpdateWorkCounter = 0;
		FFlecsEntityHandle PostUpdateWorkSystem = FlecsWorld->World.system<>()
			.kind(flecs::OnUpdate)
			.each([&PostUpdateWorkCounter](flecs::iter& Iter, size_t Index)
			{
				PostUpdateWorkCounter++;
			})
			.add(FlecsWorld->GetTagEntity(FlecsTickType_PostUpdateWork).GetFlecsId());

		ASSERT_THAT(IsTrue(MainLoopSystem.IsValid()));
		ASSERT_THAT(IsTrue(PrePhysicsSystem.IsValid()));
		ASSERT_THAT(IsTrue(DuringPhysicsSystem.IsValid()));
		ASSERT_THAT(IsTrue(PostPhysicsSystem.IsValid()));
		ASSERT_THAT(IsTrue(PostUpdateWorkSystem.IsValid()));

		ASSERT_THAT(AreEqual(0, MainLoopCounter));
		ASSERT_THAT(AreEqual(0, PrePhysicsCounter));
		ASSERT_THAT(AreEqual(0, DuringPhysicsCounter));
		ASSERT_THAT(AreEqual(0, PostPhysicsCounter));
		ASSERT_THAT(AreEqual(0, PostUpdateWorkCounter));

		Fixture->Fixture.TickWorld();

		ASSERT_THAT(AreEqual(1, MainLoopCounter));
		ASSERT_THAT(AreEqual(1, PrePhysicsCounter));
		ASSERT_THAT(AreEqual(1, DuringPhysicsCounter));
		ASSERT_THAT(AreEqual(1, PostPhysicsCounter));
		ASSERT_THAT(AreEqual(1, PostUpdateWorkCounter));

		Fixture->Fixture.TickWorld();

		ASSERT_THAT(AreEqual(2, MainLoopCounter));
		ASSERT_THAT(AreEqual(2, PrePhysicsCounter));
		ASSERT_THAT(AreEqual(2, DuringPhysicsCounter));
		ASSERT_THAT(AreEqual(2, PostPhysicsCounter));
		ASSERT_THAT(AreEqual(2, PostUpdateWorkCounter));
	}

	TEST_METHOD(A3_TickWorldWithDeltaTime)
	{
		static constexpr double TickDeltaTime = 1.0 / 30.0;
		
		int32 Counter = 0;
		FFlecsSystem TestSystem = FlecsWorld->World.system<>()
			.kind(flecs::OnUpdate)
			.each([this, &Counter](flecs::iter& Iter, size_t Index)
			{
				AddErrorIfFalse(FMath::IsNearlyEqual(Iter.delta_time(), TickDeltaTime),
					FString::Printf(TEXT("Expected delta time: %f, but got: %f"),
						TickDeltaTime,
						Iter.delta_time()
					)
				);
				
				Counter++;
			});

		ASSERT_THAT(IsTrue(TestSystem.IsValid()));
		
		ASSERT_THAT(AreEqual(0, Counter));

		Fixture->Fixture.TickWorld(TickDeltaTime);
		
		ASSERT_THAT(AreEqual(1, Counter));

		Fixture->Fixture.TickWorld(TickDeltaTime);
		
		ASSERT_THAT(AreEqual(2, Counter));
	}

	/*TEST_METHOD(A4_TickWorldThenTickWorldPausedThroughUnrealThenTickWorldUnpaused)
	{
		int32 MainLoopCounter = 0;
		FFlecsEntityHandle MainLoopSystem = FlecsWorld->World.system<>()
			.kind(flecs::OnUpdate)
			.each([&MainLoopCounter](flecs::iter& Iter, size_t Index)
			{
				MainLoopCounter++;
			});
		//.add(FlecsWorld->GetTagEntity(FlecsTickType_MainLoop).GetFlecsId());
		
		int32 PrePhysicsCounter = 0;
		FFlecsEntityHandle PrePhysicsSystem = FlecsWorld->World.system<>()
			.kind(flecs::OnUpdate)
			.each([&PrePhysicsCounter](flecs::iter& Iter, size_t Index)
			{
				PrePhysicsCounter++;
			})
			.add(FlecsWorld->GetTagEntity(FlecsTickType_PrePhysics).GetFlecsId());

		int32 DuringPhysicsCounter = 0;
		FFlecsEntityHandle DuringPhysicsSystem = FlecsWorld->World.system<>()
			.kind(flecs::OnUpdate)
			.each([&DuringPhysicsCounter](flecs::iter& Iter, size_t Index)
			{
				DuringPhysicsCounter++;
			})
			.add(FlecsWorld->GetTagEntity(FlecsTickType_DuringPhysics).GetFlecsId());

		int32 PostPhysicsCounter = 0;
		FFlecsEntityHandle PostPhysicsSystem = FlecsWorld->World.system<>()
			.kind(flecs::OnUpdate)
			.each([&PostPhysicsCounter](flecs::iter& Iter, size_t Index)
			{
				PostPhysicsCounter++;
			})
			.add(FlecsWorld->GetTagEntity(FlecsTickType_PostPhysics).GetFlecsId());

		int32 PostUpdateWorkCounter = 0;
		FFlecsEntityHandle PostUpdateWorkSystem = FlecsWorld->World.system<>()
			.kind(flecs::OnUpdate)
			.each([&PostUpdateWorkCounter](flecs::iter& Iter, size_t Index)
			{
				PostUpdateWorkCounter++;
			})
			.add(FlecsWorld->GetTagEntity(FlecsTickType_PostUpdateWork).GetFlecsId());

		ASSERT_THAT(IsTrue(MainLoopSystem.IsValid()));
		ASSERT_THAT(IsTrue(PrePhysicsSystem.IsValid()));
		ASSERT_THAT(IsTrue(DuringPhysicsSystem.IsValid()));
		ASSERT_THAT(IsTrue(PostPhysicsSystem.IsValid()));
		ASSERT_THAT(IsTrue(PostUpdateWorkSystem.IsValid()));

		ASSERT_THAT(AreEqual(0, MainLoopCounter));
		ASSERT_THAT(AreEqual(0, PrePhysicsCounter));
		ASSERT_THAT(AreEqual(0, DuringPhysicsCounter));
		ASSERT_THAT(AreEqual(0, PostPhysicsCounter));
		ASSERT_THAT(AreEqual(0, PostUpdateWorkCounter));

		Fixture->Fixture.TickWorld();

		ASSERT_THAT(AreEqual(1, MainLoopCounter));
		ASSERT_THAT(AreEqual(1, PrePhysicsCounter));
		ASSERT_THAT(AreEqual(1, DuringPhysicsCounter));
		ASSERT_THAT(AreEqual(1, PostPhysicsCounter));
		ASSERT_THAT(AreEqual(1, PostUpdateWorkCounter));

		UGameplayStatics::SetGamePaused(Fixture->Fixture.TestWorld.Get(), true);

		ASSERT_THAT(IsTrue(UGameplayStatics::IsGamePaused(Fixture->Fixture.TestWorld.Get())));

		Fixture->Fixture.TickWorld();

		ASSERT_THAT(AreEqual(2, MainLoopCounter));
		ASSERT_THAT(AreEqual(1, PrePhysicsCounter));
		ASSERT_THAT(AreEqual(1, DuringPhysicsCounter));
		ASSERT_THAT(AreEqual(1, PostPhysicsCounter));
		ASSERT_THAT(AreEqual(1, PostUpdateWorkCounter));

		UGameplayStatics::SetGamePaused(Fixture->Fixture.TestWorld.Get(), false);

		ASSERT_THAT(IsFalse(UGameplayStatics::IsGamePaused(Fixture->Fixture.TestWorld.Get())));

		Fixture->Fixture.TickWorld();

		ASSERT_THAT(AreEqual(3, MainLoopCounter));
		ASSERT_THAT(AreEqual(2, PrePhysicsCounter));
		ASSERT_THAT(AreEqual(2, DuringPhysicsCounter));
		ASSERT_THAT(AreEqual(2, PostPhysicsCounter));
		ASSERT_THAT(AreEqual(2, PostUpdateWorkCounter));
	}*/
	
}; // End of A11_UnrealFlecsWorldTickTests

#endif // WITH_AUTOMATION_TESTS
