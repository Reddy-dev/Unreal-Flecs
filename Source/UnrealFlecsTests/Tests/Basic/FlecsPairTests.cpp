// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Worlds/FlecsWorld.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS_AND_TAGS(UnrealFlecsBasicPairTests,
								   "UnrealFlecs.Components.Pairs",
							   EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter
							   | EAutomationTestFlags::CriticalPriority,
							   "[Flecs][Pair][CPP-API][StaticStruct-API][Entity-API]")
{
protected:
	virtual void OnRegisteredWorldSetUp() override
	{
		TestEntity = World()->CreateEntity("TestEntity");

		PairEntity1 = World()->CreateEntity("PairEntity1");
		PairEntity2 = World()->CreateEntity("PairEntity2");

		PairIsTagComponentHandle = World()->RegisterComponentType<FFlecsTestStruct_PairIsTag>();

		PairTestComponentHandle1 = World()->RegisterComponentType<FUSTRUCTPairTestComponent>();
		PairTestComponentHandle2 = World()->RegisterComponentType<FUSTRUCTPairTestComponent_Second>();
	}

private:
	FFlecsEntityHandle TestEntity;
	FFlecsEntityHandle PairEntity1;
	FFlecsEntityHandle PairEntity2;
	FFlecsComponentHandle PairIsTagComponentHandle;
	FFlecsComponentHandle PairTestComponentHandle1;
	FFlecsComponentHandle PairTestComponentHandle2;

public:
	TEST_METHOD(BasicPairAddRemove_Add_EntityAPI_Remove_EntityAPI)
	{
		TestEntity.AddPair(PairEntity1, PairEntity2);
		ASSERT_THAT(IsTrue(TestEntity.HasPair(PairEntity1, PairEntity2)));

		TestEntity.RemovePair(PairEntity1, PairEntity2);
		ASSERT_THAT(IsFalse(TestEntity.HasPair(PairEntity1, PairEntity2)));
	}

	TEST_METHOD(BasicPairAddRemove_Add_CPPAPI_Remove_CPPAPI)
	{
		TestEntity.AddPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>();
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));

		TestEntity.RemovePair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>();
		ASSERT_THAT(IsFalse(TestEntity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
	}

	TEST_METHOD(BasicPairAddRemove_Add_StaticStructAPI_Remove_StaticStructAPI)
	{
		TestEntity.AddPair(FUSTRUCTPairTestComponent::StaticStruct(), FUSTRUCTPairTestComponent_Second::StaticStruct());
		ASSERT_THAT(IsTrue(TestEntity.HasPair(FUSTRUCTPairTestComponent::StaticStruct(), FUSTRUCTPairTestComponent_Second::StaticStruct())));

		TestEntity.RemovePair(FUSTRUCTPairTestComponent::StaticStruct(), FUSTRUCTPairTestComponent_Second::StaticStruct());
		ASSERT_THAT(IsFalse(TestEntity.HasPair(FUSTRUCTPairTestComponent::StaticStruct(), FUSTRUCTPairTestComponent_Second::StaticStruct())));
	}

	TEST_METHOD(BasicPairAddRemove_Add_CPPAPIEntityAPI_Remove_CPPAPIEntityAPI)
	{
		TestEntity.AddPair<FUSTRUCTPairTestComponent>(PairTestComponentHandle2);
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FUSTRUCTPairTestComponent>(PairTestComponentHandle2)));
		
		TestEntity.RemovePair<FUSTRUCTPairTestComponent>(PairTestComponentHandle2);
		ASSERT_THAT(IsFalse(TestEntity.HasPair<FUSTRUCTPairTestComponent>(PairTestComponentHandle2)));
	}

	TEST_METHOD(BasicPairAddRemove_Add_CPPAPIStaticStructAPI_Remove_CPPAPIEntityAPI)
	{
		TestEntity.AddPair<FUSTRUCTPairTestComponent>(FUSTRUCTPairTestComponent::StaticStruct());
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FUSTRUCTPairTestComponent>(FUSTRUCTPairTestComponent::StaticStruct())));

		TestEntity.RemovePair<FUSTRUCTPairTestComponent>(FUSTRUCTPairTestComponent::StaticStruct());
		ASSERT_THAT(IsFalse(TestEntity.HasPair<FUSTRUCTPairTestComponent>(FUSTRUCTPairTestComponent::StaticStruct())));
	}

	TEST_METHOD(BasicPairAddRemove_Add_StaticStructAPIEntityAPI_Remove_StaticStructAPIEntityAPI)
	{
		TestEntity.AddPair(FUSTRUCTPairTestComponent::StaticStruct(), PairTestComponentHandle2);
		ASSERT_THAT(IsTrue(TestEntity.HasPair(FUSTRUCTPairTestComponent::StaticStruct(), PairTestComponentHandle2)));

		TestEntity.RemovePair(FUSTRUCTPairTestComponent::StaticStruct(), PairTestComponentHandle2);
		ASSERT_THAT(IsFalse(TestEntity.HasPair(FUSTRUCTPairTestComponent::StaticStruct(), PairTestComponentHandle2)));
	}

	TEST_METHOD(BasicPairAddSecondRemove_Add_CPPAPI_Remove_EntityAPI_SecondAPI)
	{
		TestEntity.AddPairSecond<FUSTRUCTPairTestComponent_Second>(PairTestComponentHandle1);
		ASSERT_THAT(IsTrue(TestEntity.HasPairSecond<FUSTRUCTPairTestComponent_Second>(PairTestComponentHandle1)));

		TestEntity.RemovePairSecond<FUSTRUCTPairTestComponent_Second>(PairTestComponentHandle1);
		ASSERT_THAT(IsFalse(TestEntity.HasPairSecond<FUSTRUCTPairTestComponent_Second>(PairTestComponentHandle1)));
	}

	TEST_METHOD(BasicPairAddSecondRemove_Add_CPPAPI_Remove_StaticStructAPI_SecondAPI)
	{
		TestEntity.AddPairSecond<FUSTRUCTPairTestComponent_Second>(FUSTRUCTPairTestComponent::StaticStruct());
		ASSERT_THAT(IsTrue(TestEntity.HasPairSecond<FUSTRUCTPairTestComponent_Second>(FUSTRUCTPairTestComponent::StaticStruct())));

		TestEntity.RemovePairSecond<FUSTRUCTPairTestComponent_Second>(FUSTRUCTPairTestComponent::StaticStruct());
		ASSERT_THAT(IsFalse(TestEntity.HasPairSecond<FUSTRUCTPairTestComponent_Second>(FUSTRUCTPairTestComponent::StaticStruct())));
	}

	TEST_METHOD(BasicPairAddRemove_Add_StaticStructAPIEntityAPI_Remove_CPPAPIEntityAPI)
	{
		TestEntity.AddPair(FUSTRUCTPairTestComponent::StaticStruct(), PairTestComponentHandle2);
		ASSERT_THAT(IsTrue(TestEntity.HasPair(FUSTRUCTPairTestComponent::StaticStruct(), PairTestComponentHandle2)));

		TestEntity.RemovePair<FUSTRUCTPairTestComponent>(PairTestComponentHandle2);
		ASSERT_THAT(IsFalse(TestEntity.HasPair(FUSTRUCTPairTestComponent::StaticStruct(), PairTestComponentHandle2)));
	}

}; // UnrealFlecsBasicPairTests

#endif // #if WITH_AUTOMATION_TESTS
