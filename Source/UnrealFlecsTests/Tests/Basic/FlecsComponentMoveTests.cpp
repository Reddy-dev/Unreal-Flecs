// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Worlds/FlecsWorld.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsComponentMoveTests,
								 "UnrealFlecs.Components.Operations.Move",
                               EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter,
                               "[Flecs][Entity][Tag][Component]")
{
protected:
	virtual void OnRegisteredWorldSetUp() override
	{
		TagEntity = World()->RegisterComponentType<FFlecsTestStruct_Tag>();
		ValuedComponentEntity = World()->RegisterComponentType<FFlecsTestStruct_Value>();

		ToggleableComponentHandle = World()->RegisterComponentType<FFlecsTestStruct_Toggleable>();

		TestEntity = World()->CreateEntity("TestEntity");
		ToggleableEntityTest = World()->CreateEntity("ToggleableEntityTest")
			.Add<FFlecsTestStruct_Toggleable>();

		Query = World()->GetNativeFlecsWorld().query_builder<FFlecsTestStruct_Value>()
			.cached()
			.detect_changes()
			.build();
	}

	virtual void OnWorldTearDown() override
	{
		Query.destruct();
		Query = {};
	}

private:
	FFlecsEntityHandle TagEntity;
	FFlecsEntityHandle ValuedComponentEntity;
	FFlecsComponentHandle ToggleableComponentHandle;
	FFlecsEntityHandle TestEntity;
	FFlecsEntityHandle ToggleableEntityTest;
	flecs::query<FFlecsTestStruct_Value> Query;

public:
	TEST_METHOD(BasicMovableComponent_MoveUSTRUCT_MovableComponent)
	{
		ASSERT_THAT(IsTrue(FUStructTestComponent_MovableUSTRUCT::StaticStruct()->GetCppStructOps()->HasMoveAssign()));
		
		World()->RegisterComponentType(FUStructTestComponent_MovableUSTRUCT::StaticStruct());
		
		FUStructTestComponent_MovableUSTRUCT InitialValue;
		InitialValue.Name = TEXT("InitialName");

		TestEntity.Set<FUStructTestComponent_MovableUSTRUCT>(InitialValue);
		ASSERT_THAT(IsTrue(TestEntity.Has<FUStructTestComponent_MovableUSTRUCT>()));
		TestEntity.Add<FFlecsTestStruct_Toggleable>(); // this will move the previous component in memory
		ASSERT_THAT(IsTrue(TestEntity.Has<FUStructTestComponent_MovableUSTRUCT>()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Toggleable>()));
	}

	TEST_METHOD(BasicMovableComponent_MoveUSTRUCT_NoMovableRegisteredComponent)
	{
		ASSERT_THAT(IsFalse(FUStructTestComponent_MovableNotRegisteredUSTRUCT::StaticStruct()->GetCppStructOps()->HasMoveAssign()));
		World()->RegisterComponentType(FUStructTestComponent_MovableNotRegisteredUSTRUCT::StaticStruct());
		
		FUStructTestComponent_MovableNotRegisteredUSTRUCT InitialValue;
		InitialValue.Name = TEXT("InitialName");

		TestEntity.Set<FUStructTestComponent_MovableNotRegisteredUSTRUCT>(InitialValue);
		ASSERT_THAT(IsTrue(TestEntity.Has<FUStructTestComponent_MovableNotRegisteredUSTRUCT>()));
		TestEntity.Add<FFlecsTestStruct_Toggleable>(); // this will move the previous component in memory
		ASSERT_THAT(IsTrue(TestEntity.Has<FUStructTestComponent_MovableNotRegisteredUSTRUCT>()));
	}

	TEST_METHOD(BasicMovableComponent_MoveLifecycleTracker_MovableRegistered)
	{
		ASSERT_THAT(IsTrue(FUStructTestComponent_LifecycleTracker::StaticStruct()->GetCppStructOps()->HasMoveAssign()));
		World()->RegisterComponentType(FUStructTestComponent_LifecycleTracker::StaticStruct());
		
		FUStructTestComponent_LifecycleTracker Initial;
		TestEntity.Set<FUStructTestComponent_LifecycleTracker>(Initial);

		ASSERT_THAT(IsTrue(TestEntity.Has<FUStructTestComponent_LifecycleTracker>()));
		
		TestEntity.Add<FFlecsTestStruct_Toggleable>();
		
		ASSERT_THAT(IsTrue(TestEntity.Has<FUStructTestComponent_LifecycleTracker>()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Toggleable>()));
		
		const FUStructTestComponent_LifecycleTracker& Tracker
			= TestEntity.Get<FUStructTestComponent_LifecycleTracker>();
		
		ASSERT_THAT(IsTrue(Tracker.MovedInto()));
		
		// any move/copy *assignments* counted on the destination
		ASSERT_THAT(IsTrue(Tracker.TimesMoveAssignedInto == 0));
	}

	TEST_METHOD(BasicMovableComponent_MoveLifecycleTracker_NoMoveRegistration)
	{
		ASSERT_THAT(IsFalse(FFlecsTestStruct_LifecycleTracker_NoMoveReg::StaticStruct()->GetCppStructOps()->HasMoveAssign()));
		
		World()->RegisterComponentType(FFlecsTestStruct_LifecycleTracker_NoMoveReg::StaticStruct());
		
		FFlecsTestStruct_LifecycleTracker_NoMoveReg Initial;
		TestEntity.Set<FFlecsTestStruct_LifecycleTracker_NoMoveReg>(Initial);

		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_LifecycleTracker_NoMoveReg>()));

		TestEntity.Add<FFlecsTestStruct_Toggleable>();

		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_LifecycleTracker_NoMoveReg>()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Toggleable>()));

		const FFlecsTestStruct_LifecycleTracker_NoMoveReg& Tracker
			= TestEntity.Get<FFlecsTestStruct_LifecycleTracker_NoMoveReg>();
		
		ASSERT_THAT(IsFalse(Tracker.MovedInto()));
		
		ASSERT_THAT(IsTrue(Tracker.TimesMoveAssignedInto == 0));
	}

}; // FlecsComponentMoveTests


#endif // #if WITH_AUTOMATION_TESTS
