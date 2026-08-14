// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Entities/FlecsEntityRange.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(UnrealFlecsEntityTests,
								   "UnrealFlecs.Entities.Core",
							   EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
								| EAutomationTestFlags::CriticalPriority, "[Flecs]")
{
	TEST_METHOD(SpawnEmptyEntity)
	{
		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsFalse(TestEntity.HasName()));
	}

	TEST_METHOD(SpawnNamedEntity)
	{
		static const FString EntityName = TEXT("MyTestEntity");

		const FFlecsEntityHandle TestEntity = World()->CreateEntity(EntityName);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasName()));
		ASSERT_THAT(AreEqual(EntityName, TestEntity.GetName()));
	}

	TEST_METHOD(SpawnEntityWithEmptyName)
	{
		const FFlecsEntityHandle TestEntity = World()->CreateEntity(TEXT(""));
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsFalse(TestEntity.HasName()));
	}

	TEST_METHOD(SpawnEntityWithNameWithDefaultSeparator)
	{
		static const FString EntityName = TEXT("My::Test::Entity");

		const FFlecsEntityHandle TestEntity = World()->CreateEntity(EntityName);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasName()));
		
		ASSERT_THAT(AreEqual(TEXT("Entity"), TestEntity.GetName()));
		ASSERT_THAT(AreEqual(TEXT("::My::Test::Entity"), TestEntity.GetPath()));
		ASSERT_THAT(AreEqual(TEXT("My.Test.Entity"), TestEntity.GetPath(".", "")));
		ASSERT_THAT(AreEqual(TEXT(".My.Test.Entity"), TestEntity.GetPath(".", ".")));
		ASSERT_THAT(AreEqual(TEXT("My/Test/Entity"), TestEntity.GetPath("/", "")));
		ASSERT_THAT(AreEqual(TEXT("/My/Test/Entity"), TestEntity.GetPath("/", "/")));
	}

	TEST_METHOD(SpawnEntityWithName_WithCustomSeparator)
	{
		static const FString EntityName = TEXT("My/Custom/Separator/Entity");

		const FFlecsEntityHandle TestEntity = World()->CreateEntity(EntityName, TEXT("/"));
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasName()));
		
		ASSERT_THAT(AreEqual(TEXT("Entity"), TestEntity.GetName()));
		ASSERT_THAT(AreEqual(TEXT("My/Custom/Separator/Entity"), TestEntity.GetPath("/", "")));
		ASSERT_THAT(AreEqual(TEXT("/My/Custom/Separator/Entity"), TestEntity.GetPath("/", "/")));
		ASSERT_THAT(AreEqual(TEXT("My.Custom.Separator.Entity"), TestEntity.GetPath(".", "")));
		ASSERT_THAT(AreEqual(TEXT(".My.Custom.Separator.Entity"), TestEntity.GetPath(".", ".")));
		ASSERT_THAT(AreEqual(TEXT("::My.Custom.Separator.Entity"), TestEntity.GetPath(".", "::")));
		ASSERT_THAT(AreEqual(TEXT("::My::Custom::Separator::Entity"), TestEntity.GetPath()));
	}

	TEST_METHOD(SpawnEntityWithParent_SetChildOf_API)
	{
		const FFlecsEntityHandle ParentEntity = World()->CreateEntity("ParentEntity");
		const FFlecsEntityHandle ChildEntity = World()->CreateEntity("ChildEntity")
			.SetChildOf(ParentEntity);

		ASSERT_THAT(IsTrue(ChildEntity.IsValid()));
		ASSERT_THAT(IsTrue(ChildEntity.HasParent()));
		ASSERT_THAT(AreEqual(ParentEntity, ChildEntity.GetParent<FFlecsEntityHandle>()));
	}
	
	TEST_METHOD(SpawnEntityWithParent_SetParent_API)
	{
		const FFlecsEntityHandle ParentEntity = World()->CreateEntity("ParentEntity");
		const FFlecsEntityHandle ChildEntity = World()->CreateEntity("ChildEntity")
			.SetParent(ParentEntity);

		ASSERT_THAT(IsTrue(ChildEntity.IsValid()));
		ASSERT_THAT(IsTrue(ChildEntity.HasParent()));
		ASSERT_THAT(AreEqual(ParentEntity, ChildEntity.GetParent<FFlecsEntityHandle>()));
	}
	
	TEST_METHOD(SpawnEntityWithComponent_GetNComponents_API)
	{
		World()->RegisterComponentType<FFlecsTest_CPPStructValue>();
		World()->RegisterComponentType<FFlecsTestStruct_Value>();
		
		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity")
			.Set<FFlecsTest_CPPStructValue>({ 42 })
			.Set<FFlecsTestStruct_Value>({ 100 });

		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTest_CPPStructValue>()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		
		auto [CPPStructValue, UStructValue] 
			= TestEntity.GetN<FFlecsTest_CPPStructValue, FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(CPPStructValue.Value == 42));
		ASSERT_THAT(IsTrue(UStructValue.Value == 100));
	}
	
	TEST_METHOD(SpawnEntityWithChildrenInOrder_SetChildOrder_C_API)
	{
		const FFlecsEntityHandle ParentEntity = World()->CreateEntity("ParentEntity")
			.Add(flecs::OrderedChildren);
		ASSERT_THAT(IsTrue(ParentEntity.IsValid()));
		ASSERT_THAT(IsTrue(ParentEntity.Has(flecs::OrderedChildren)));

		const FFlecsEntityHandle ChildEntityA = World()->CreateEntity("ChildEntityA").SetChildOf(ParentEntity);
		const FFlecsEntityHandle ChildEntityB = World()->CreateEntity("ChildEntityB").SetChildOf(ParentEntity);
		const FFlecsEntityHandle ChildEntityC = World()->CreateEntity("ChildEntityC").SetChildOf(ParentEntity);
		
		ASSERT_THAT(IsTrue(ChildEntityA.HasPair(flecs::ChildOf, ParentEntity.GetFlecsId()) && !ChildEntityA.Has<flecs::Parent>()));
		ASSERT_THAT(IsTrue(ChildEntityB.HasPair(flecs::ChildOf, ParentEntity.GetFlecsId()) && !ChildEntityB.Has<flecs::Parent>()));
		ASSERT_THAT(IsTrue(ChildEntityC.HasPair(flecs::ChildOf, ParentEntity.GetFlecsId()) && !ChildEntityC.Has<flecs::Parent>()));

		{
			TArray<FFlecsEntityHandle> ChildrenArray;
			ParentEntity.IterateChildren([&ChildrenArray](const FFlecsEntityHandle& InChildEntity)
			{
				ChildrenArray.Add(InChildEntity);
			});

			ASSERT_THAT(IsTrue(ChildrenArray.Num() == 3));
			ASSERT_THAT(AreEqual(ChildrenArray[0], ChildEntityA));
			ASSERT_THAT(AreEqual(ChildrenArray[1], ChildEntityB));
			ASSERT_THAT(AreEqual(ChildrenArray[2], ChildEntityC));
		}

		FFlecsId NewChildrenEntityOrder[] = { ChildEntityC.GetFlecsId(), ChildEntityA.GetFlecsId(), ChildEntityB.GetFlecsId() };
		ParentEntity.SetChildOrder(NewChildrenEntityOrder, 3);

		{
			TArray<FFlecsEntityHandle> ChildrenArray;
			ParentEntity.IterateChildren([&ChildrenArray](const FFlecsEntityHandle& InChildEntity)
			{
				ChildrenArray.Add(InChildEntity);
			});

			ASSERT_THAT(IsTrue(ChildrenArray.Num() == 3));
			ASSERT_THAT(AreEqual(ChildrenArray[0], ChildEntityC));
			ASSERT_THAT(AreEqual(ChildrenArray[1], ChildEntityA));
			ASSERT_THAT(AreEqual(ChildrenArray[2], ChildEntityB));
		}
	}
	
	TEST_METHOD(SpawnEntityWithChildrenInOrder_SetChildOrder_TArrayView_API)
	{
		const FFlecsEntityHandle ParentEntity = World()->CreateEntity("ParentEntity")
			.Add(flecs::OrderedChildren);
		ASSERT_THAT(IsTrue(ParentEntity.IsValid()));
		ASSERT_THAT(IsTrue(ParentEntity.Has(flecs::OrderedChildren)));

		const FFlecsEntityHandle ChildEntityA = World()->CreateEntity("ChildEntityA").SetChildOf(ParentEntity);
		const FFlecsEntityHandle ChildEntityB = World()->CreateEntity("ChildEntityB").SetChildOf(ParentEntity);
		const FFlecsEntityHandle ChildEntityC = World()->CreateEntity("ChildEntityC").SetChildOf(ParentEntity);
		
		ASSERT_THAT(IsTrue(ChildEntityA.HasPair(flecs::ChildOf, ParentEntity.GetFlecsId()) && !ChildEntityA.Has<flecs::Parent>()));
		ASSERT_THAT(IsTrue(ChildEntityB.HasPair(flecs::ChildOf, ParentEntity.GetFlecsId()) && !ChildEntityB.Has<flecs::Parent>()));
		ASSERT_THAT(IsTrue(ChildEntityC.HasPair(flecs::ChildOf, ParentEntity.GetFlecsId()) && !ChildEntityC.Has<flecs::Parent>()));

		{
			TArray<FFlecsEntityHandle> ChildrenArray;
			ParentEntity.IterateChildren([&ChildrenArray](const FFlecsEntityHandle& InChildEntity)
			{
				ChildrenArray.Add(InChildEntity);
			});

			ASSERT_THAT(IsTrue(ChildrenArray.Num() == 3));
			ASSERT_THAT(AreEqual(ChildrenArray[0], ChildEntityA));
			ASSERT_THAT(AreEqual(ChildrenArray[1], ChildEntityB));
			ASSERT_THAT(AreEqual(ChildrenArray[2], ChildEntityC));
		}

		TArray<FFlecsId> NewChildrenEntityOrder = TArray<FFlecsId>{ ChildEntityC.GetFlecsId(), ChildEntityA.GetFlecsId(), ChildEntityB.GetFlecsId() };
		ParentEntity.SetChildOrder(NewChildrenEntityOrder);

		{
			TArray<FFlecsEntityHandle> ChildrenArray;
			ParentEntity.IterateChildren([&ChildrenArray](const FFlecsEntityHandle& InChildEntity)
			{
				ChildrenArray.Add(InChildEntity);
			});

			ASSERT_THAT(IsTrue(ChildrenArray.Num() == 3));
			ASSERT_THAT(AreEqual(ChildrenArray[0], ChildEntityC));
			ASSERT_THAT(AreEqual(ChildrenArray[1], ChildEntityA));
			ASSERT_THAT(AreEqual(ChildrenArray[2], ChildEntityB));
		}
	}
	
	
	TEST_METHOD(SpawnEntityWithDontFragmentChildrenInOrder_SetChildOrder_C_API)
	{
		const FFlecsEntityHandle ParentEntity = World()->CreateEntity("ParentEntity")
			.Add(flecs::OrderedChildren);
		ASSERT_THAT(IsTrue(ParentEntity.IsValid()));
		ASSERT_THAT(IsTrue(ParentEntity.Has(flecs::OrderedChildren)));

		const FFlecsEntityHandle ChildEntityA = World()->CreateEntity("ChildEntityA").SetParent(ParentEntity);
		const FFlecsEntityHandle ChildEntityB = World()->CreateEntity("ChildEntityB").SetParent(ParentEntity);
		const FFlecsEntityHandle ChildEntityC = World()->CreateEntity("ChildEntityC").SetParent(ParentEntity);
		
		ASSERT_THAT(IsTrue(ChildEntityA.Has<flecs::Parent>() && ChildEntityA.HasPair(flecs::ChildOf, flecs::Wildcard)));
		ASSERT_THAT(IsTrue(ChildEntityB.Has<flecs::Parent>() && ChildEntityB.HasPair(flecs::ChildOf, flecs::Wildcard)));
		ASSERT_THAT(IsTrue(ChildEntityC.Has<flecs::Parent>() && ChildEntityC.HasPair(flecs::ChildOf, flecs::Wildcard)));

		{
			TArray<FFlecsEntityHandle> ChildrenArray;
			ParentEntity.IterateChildren([&ChildrenArray](const FFlecsEntityHandle& InChildEntity)
			{
				ChildrenArray.Add(InChildEntity);
			});

			ASSERT_THAT(IsTrue(ChildrenArray.Num() == 3));
			ASSERT_THAT(AreEqual(ChildrenArray[0], ChildEntityA));
			ASSERT_THAT(AreEqual(ChildrenArray[1], ChildEntityB));
			ASSERT_THAT(AreEqual(ChildrenArray[2], ChildEntityC));
		}

		FFlecsId NewChildrenEntityOrder[] = { ChildEntityC.GetFlecsId(), ChildEntityA.GetFlecsId(), ChildEntityB.GetFlecsId() };
		ParentEntity.SetChildOrder(NewChildrenEntityOrder, 3);

		{
			TArray<FFlecsEntityHandle> ChildrenArray;
			ParentEntity.IterateChildren([&ChildrenArray](const FFlecsEntityHandle& InChildEntity)
			{
				ChildrenArray.Add(InChildEntity);
			});

			ASSERT_THAT(IsTrue(ChildrenArray.Num() == 3));
			ASSERT_THAT(AreEqual(ChildrenArray[0], ChildEntityC));
			ASSERT_THAT(AreEqual(ChildrenArray[1], ChildEntityA));
			ASSERT_THAT(AreEqual(ChildrenArray[2], ChildEntityB));
		}
		
	}

	TEST_METHOD(SpawnEntityWithDontFragmentChildrenInOrder_SetChildOrder_TArrayView_API)
	{
		const FFlecsEntityHandle ParentEntity = World()->CreateEntity("ParentEntity")
			.Add(flecs::OrderedChildren);
		ASSERT_THAT(IsTrue(ParentEntity.IsValid()));
		ASSERT_THAT(IsTrue(ParentEntity.Has(flecs::OrderedChildren)));

		const FFlecsEntityHandle ChildEntityA = World()->CreateEntity("ChildEntityA").SetParent(ParentEntity);
		const FFlecsEntityHandle ChildEntityB = World()->CreateEntity("ChildEntityB").SetParent(ParentEntity);
		const FFlecsEntityHandle ChildEntityC = World()->CreateEntity("ChildEntityC").SetParent(ParentEntity);
		
		ASSERT_THAT(IsTrue(ChildEntityA.Has<flecs::Parent>() && ChildEntityA.HasPair(flecs::ChildOf, flecs::Wildcard)));
		ASSERT_THAT(IsTrue(ChildEntityB.Has<flecs::Parent>() && ChildEntityB.HasPair(flecs::ChildOf, flecs::Wildcard)));
		ASSERT_THAT(IsTrue(ChildEntityC.Has<flecs::Parent>() && ChildEntityC.HasPair(flecs::ChildOf, flecs::Wildcard)));

		{
			TArray<FFlecsEntityHandle> ChildrenArray;
			ParentEntity.IterateChildren([&ChildrenArray](const FFlecsEntityHandle& InChildEntity)
			{
				ChildrenArray.Add(InChildEntity);
			});

			ASSERT_THAT(IsTrue(ChildrenArray.Num() == 3));
			ASSERT_THAT(AreEqual(ChildrenArray[0], ChildEntityA));
			ASSERT_THAT(AreEqual(ChildrenArray[1], ChildEntityB));
			ASSERT_THAT(AreEqual(ChildrenArray[2], ChildEntityC));
		}

		TArray<FFlecsId> NewChildrenEntityOrder = TArray<FFlecsId>{ ChildEntityC.GetFlecsId(), ChildEntityA.GetFlecsId(), ChildEntityB.GetFlecsId() };
		ParentEntity.SetChildOrder(NewChildrenEntityOrder);

		{
			TArray<FFlecsEntityHandle> ChildrenArray;
			ParentEntity.IterateChildren([&ChildrenArray](const FFlecsEntityHandle& InChildEntity)
			{
				ChildrenArray.Add(InChildEntity);
			});

			ASSERT_THAT(IsTrue(ChildrenArray.Num() == 3));
			ASSERT_THAT(AreEqual(ChildrenArray[0], ChildEntityC));
			ASSERT_THAT(AreEqual(ChildrenArray[1], ChildEntityA));
			ASSERT_THAT(AreEqual(ChildrenArray[2], ChildEntityB));
		}
	}

	TEST_METHOD(CreateEntityRange_ReturnsTrackedUObject)
	{
		const int32 RangeMinimum = static_cast<int32>(World()->GetMaxId().GetIndex()) + 1000;
		const int32 RangeMaximum = RangeMinimum + 9;

		UFlecsEntityRange* EntityRange = World()->CreateEntityRange("Test", RangeMinimum, RangeMaximum);

		ASSERT_THAT(IsTrue(IsValid(EntityRange)));
		ASSERT_THAT(AreEqual(RangeMinimum, EntityRange->GetMinimum()));
		ASSERT_THAT(AreEqual(RangeMaximum, EntityRange->GetMaximum()));
		ASSERT_THAT(IsTrue(World() == EntityRange->GetTypedOuter<UFlecsWorld>()));
		ASSERT_THAT(IsTrue(World()->GetEntityRanges().Contains(EntityRange)));
	}

	TEST_METHOD(SetActiveEntityRange_AllocatesEntitiesInsideRange)
	{
		const int32 RangeMinimum = static_cast<int32>(World()->GetMaxId().GetIndex()) + 1000;
		const int32 RangeMaximum = RangeMinimum + 2;
		
		UFlecsEntityRange* EntityRange = World()->CreateEntityRange("Test", RangeMinimum, RangeMaximum);
		ASSERT_THAT(IsTrue(IsValid(EntityRange)));
		
		World()->SetActiveEntityRange(EntityRange);

		const FFlecsEntityHandle FirstEntity = World()->CreateEntity("RangeEntityA");
		const FFlecsEntityHandle SecondEntity = World()->CreateEntity("RangeEntityB");
		
		ASSERT_THAT(IsTrue(EntityRange == World()->GetActiveEntityRange()));
		ASSERT_THAT(IsTrue(RangeMinimum == static_cast<int32>(FirstEntity.GetFlecsId().GetIndex())));
		ASSERT_THAT(IsTrue(RangeMinimum + 1 == static_cast<int32>(SecondEntity.GetFlecsId().GetIndex())));
	}
	
}; // UnrealFlecsComponentRegistrationTests


#endif // #if WITH_AUTOMATION_TESTS
