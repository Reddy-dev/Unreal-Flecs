// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Pipelines/FlecsTickTypeNativeTags.h"
#include "Worlds/FlecsWorld.h"

FLECS_TEST_CLASS_WITH_FLAGS(UnrealFlecsWorldTickTests,
								   "UnrealFlecs.World.Tick",
							   EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
								| EAutomationTestFlags::CriticalPriority)
{
protected:
	virtual EWorldType::Type WorldType() const override
	{
		return EWorldType::Game;
	}

public:
	TEST_METHOD(CreateSystemsInMainLoopAndTickWorld)
	{
		static constexpr double TickDeltaTime = 1.0 / 60.0;
		
		int32 Counter = 0;
		flecs::system TestSystem = World()->GetNativeFlecsWorld().system<>()
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
		flecs::system TestSystemPostUpdate = World()->GetNativeFlecsWorld().system<>()
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
		
		ASSERT_THAT(IsTrue(TestSystem.is_valid()));
		ASSERT_THAT(IsTrue(TestSystemPostUpdate.is_valid()));
		
		ASSERT_THAT(AreEqual(0, Counter));
		ASSERT_THAT(AreEqual(0, PostUpdateCounter));

		TickWorld(TickDeltaTime);
		
		ASSERT_THAT(AreEqual(1, Counter));
		ASSERT_THAT(AreEqual(1, PostUpdateCounter));

		TickWorld(TickDeltaTime);
		
		ASSERT_THAT(AreEqual(2, Counter));
		ASSERT_THAT(AreEqual(2, PostUpdateCounter));
	}

	TEST_METHOD(CreateSystemsInUnrealTickTypesAndMainLoopAndTickWorld)
	{
		// @TODO: use FFlecsSystem instead of FFlecsEntityHandle

		int32 MainLoopCounter = 0;
		FFlecsEntityHandle MainLoopSystem = World()->GetNativeFlecsWorld().system<>()
			.kind(flecs::OnUpdate)
			.each([&MainLoopCounter](flecs::iter& Iter, size_t Index)
			{
				MainLoopCounter++;
			});
			//.add(World()->GetTagEntity(FlecsTickType_MainLoop).GetFlecsId());
		
		int32 PrePhysicsCounter = 0;
		FFlecsEntityHandle PrePhysicsSystem = World()->GetNativeFlecsWorld().system<>()
			.kind(flecs::OnUpdate)
			.each([&PrePhysicsCounter](flecs::iter& Iter, size_t Index)
			{
				PrePhysicsCounter++;
			})
			.add(World()->GetTagEntity(FlecsTickType_PrePhysics).GetFlecsId());

		int32 DuringPhysicsCounter = 0;
		FFlecsEntityHandle DuringPhysicsSystem = World()->GetNativeFlecsWorld().system<>()
			.kind(flecs::OnUpdate)
			.each([&DuringPhysicsCounter](flecs::iter& Iter, size_t Index)
			{
				DuringPhysicsCounter++;
			})
			.add(World()->GetTagEntity(FlecsTickType_DuringPhysics).GetFlecsId());

		int32 PostPhysicsCounter = 0;
		FFlecsEntityHandle PostPhysicsSystem = World()->GetNativeFlecsWorld().system<>()
			.kind(flecs::OnUpdate)
			.each([&PostPhysicsCounter](flecs::iter& Iter, size_t Index)
			{
				PostPhysicsCounter++;
			})
			.add(World()->GetTagEntity(FlecsTickType_PostPhysics).GetFlecsId());

		int32 PostUpdateWorkCounter = 0;
		FFlecsEntityHandle PostUpdateWorkSystem = World()->GetNativeFlecsWorld().system<>()
			.kind(flecs::OnUpdate)
			.each([&PostUpdateWorkCounter](flecs::iter& Iter, size_t Index)
			{
				PostUpdateWorkCounter++;
			})
			.add(World()->GetTagEntity(FlecsTickType_PostUpdateWork).GetFlecsId());

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

		TickWorld();

		ASSERT_THAT(AreEqual(1, MainLoopCounter));
		ASSERT_THAT(AreEqual(1, PrePhysicsCounter));
		ASSERT_THAT(AreEqual(1, DuringPhysicsCounter));
		ASSERT_THAT(AreEqual(1, PostPhysicsCounter));
		ASSERT_THAT(AreEqual(1, PostUpdateWorkCounter));

		TickWorld();

		ASSERT_THAT(AreEqual(2, MainLoopCounter));
		ASSERT_THAT(AreEqual(2, PrePhysicsCounter));
		ASSERT_THAT(AreEqual(2, DuringPhysicsCounter));
		ASSERT_THAT(AreEqual(2, PostPhysicsCounter));
		ASSERT_THAT(AreEqual(2, PostUpdateWorkCounter));
	}

	TEST_METHOD(TickWorldWithDeltaTime)
	{
		static constexpr double TickDeltaTime = 1.0 / 30.0;
		
		int32 Counter = 0;
		flecs::system TestSystem = World()->GetNativeFlecsWorld().system<>()
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

		ASSERT_THAT(IsTrue(TestSystem.is_valid()));
		
		ASSERT_THAT(AreEqual(0, Counter));

		TickWorld(TickDeltaTime);
		
		ASSERT_THAT(AreEqual(1, Counter));

		TickWorld(TickDeltaTime);
		
		ASSERT_THAT(AreEqual(2, Counter));
	}
	
}; // UnrealFlecsWorldTickTests

#endif // WITH_AUTOMATION_TESTS
