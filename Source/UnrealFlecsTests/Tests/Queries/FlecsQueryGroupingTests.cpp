// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "FlecsQueryDefinitionTestTypes.h"
#include "Queries/FlecsQuery.h"
#include "Queries/FlecsQueryBuilderView.h"

#include "Queries/FlecsQueryDefinition.h"
#include "Queries/Generator/FlecsQueryGeneratorInputType.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS(FlecsQueryGroupingTests,
								   "UnrealFlecs.Queries.Grouping",
							   EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
								| EAutomationTestFlags::CriticalPriority)
{
protected:
	virtual void OnRegisteredWorldSetUp() override
	{
		World()->RegisterComponentType(FFlecsTestStruct_PairIsTag::StaticStruct());
		World()->RegisterComponentType<ETestEnum>();
		World()->RegisterComponentType(StaticEnum<EFlecsTestEnum_UENUM>());
	}

public:
	TEST_METHOD(GroupBy_SetGroup_ScriptStructValue_ScriptStructAPI)
	{
		const FFlecsEntityHandle Relation = World()->CreateEntity();
		const FFlecsEntityHandle GroupA = World()->CreateEntity();
		const FFlecsEntityHandle GroupB = World()->CreateEntity();
		
		const TTypedFlecsQuery<FFlecsTestStruct_Value> Query = World()->CreateQueryBuilder<const FFlecsTestStruct_Value>()
			.GroupBy(Relation)
			.Build();
		
		FFlecsEntityHandle TestEntity1 = World()->CreateEntity()
			.AddPair(Relation, GroupA)
			.Set<FFlecsTestStruct_Value>({ 1 });
		
		FFlecsEntityHandle TestEntity2 = World()->CreateEntity()
			.AddPair(Relation, GroupB)
			.Set<FFlecsTestStruct_Value>({ 20 });
		
		int32 Count = 0;
		Query.set_group(GroupB).each([&](flecs::iter& Iter, size_t IndexInIter, const FFlecsTestStruct_Value Value)
		{
			ASSERT_THAT(IsTrue(Value.Value == 20));
			Count++;
		});
			
		ASSERT_THAT(IsTrue(Count == 1));
	}

	TEST_METHOD(GroupByCallbackDefinition_ClassifiesTablesByTag_ScriptStructAPI)
	{
		const TTypedFlecsQuery<FFlecsTestStruct_Value> Query = World()->CreateQueryBuilder<const FFlecsTestStruct_Value>()
			.GroupByCallbackDefinition(
				FFlecsTestStruct_Value::StaticStruct(),
				TInstancedStruct<FFlecsQueryGroupByCallbackDefinitionTest_TableHasTag>::Make())
			.Build();

		World()->CreateEntity()
			.Set<FFlecsTestStruct_Value>({ 10 });

		World()->CreateEntity()
			.Add<FFlecsTestStruct_Tag>()
			.Set<FFlecsTestStruct_Value>({ 20 });

		int32 WithoutTagCount = 0;
		Query.set_group(UE::Flecs::Tests::GroupByWithoutTagGroupId).each(
			[&](flecs::iter& Iter, size_t IndexInIter, const FFlecsTestStruct_Value& Value)
		{
			ASSERT_THAT(IsTrue(Value.Value == 10));
			WithoutTagCount++;
		});

		int32 WithTagCount = 0;
		Query.set_group(UE::Flecs::Tests::GroupByWithTagGroupId).each(
			[&](flecs::iter& Iter, size_t IndexInIter, const FFlecsTestStruct_Value& Value)
		{
			ASSERT_THAT(IsTrue(Value.Value == 20));
			WithTagCount++;
		});

		ASSERT_THAT(IsTrue(WithoutTagCount == 1));
		ASSERT_THAT(IsTrue(WithTagCount == 1));
	}

	TEST_METHOD(GroupByInlineCallback_ClassifiesTablesByTag_ScriptStructAPI)
	{
		const TTypedFlecsQuery<FFlecsTestStruct_Value> Query = World()->CreateQueryBuilder<const FFlecsTestStruct_Value>()
			.GroupBy(
				FFlecsTestStruct_Value::StaticStruct(),
				[](const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
					FFlecsTableHandle InTable,
					FFlecsId InId,
					void* InContext) -> uint64
				{
					return UE::Flecs::Tests::GroupByTableHasTag(InWorld, InTable, InId, InContext);
				})
			.Build();

		World()->CreateEntity()
			.Set<FFlecsTestStruct_Value>({ 30 });

		World()->CreateEntity()
			.Add<FFlecsTestStruct_Tag>()
			.Set<FFlecsTestStruct_Value>({ 40 });

		int32 WithoutTagCount = 0;
		Query.set_group(UE::Flecs::Tests::GroupByWithoutTagGroupId).each(
			[&](flecs::iter& Iter, size_t IndexInIter, const FFlecsTestStruct_Value& Value)
		{
			ASSERT_THAT(IsTrue(Value.Value == 30));
			WithoutTagCount++;
		});

		int32 WithTagCount = 0;
		Query.set_group(UE::Flecs::Tests::GroupByWithTagGroupId).each(
			[&](flecs::iter& Iter, size_t IndexInIter, const FFlecsTestStruct_Value& Value)
		{
			ASSERT_THAT(IsTrue(Value.Value == 40));
			WithTagCount++;
		});

		ASSERT_THAT(IsTrue(WithoutTagCount == 1));
		ASSERT_THAT(IsTrue(WithTagCount == 1));
	}

	TEST_METHOD(GroupByLifecycleCallbacks_CreateAndDestroyGroupContexts_ScriptStructAPI)
	{
		UE::Flecs::Tests::ResetGroupByLifecycleLog();

		FFlecsQuery Query = World()->CreateQueryBuilder()
			.With(FFlecsTestStruct_Value::StaticStruct())
			.GroupByCallbackDefinition(
				FFlecsTestStruct_Value::StaticStruct(),
				TInstancedStruct<FFlecsQueryGroupByCallbackDefinitionTest_TableHasTag>::Make())
			.OnGroupCreated(UE::Flecs::Tests::RecordGroupCreated)
			.OnGroupDestroyed(UE::Flecs::Tests::RecordGroupDestroyed)
			.Build();

		World()->CreateEntity()
			.Set<FFlecsTestStruct_Value>({ 50 });

		World()->CreateEntity()
			.Add<FFlecsTestStruct_Tag>()
			.Set<FFlecsTestStruct_Value>({ 60 });

		ASSERT_THAT(IsTrue(Query.count() == 2));

		ASSERT_THAT(IsTrue(UE::Flecs::Tests::GetCreatedGroupIds().Contains(UE::Flecs::Tests::GroupByWithoutTagGroupId)));
		ASSERT_THAT(IsTrue(UE::Flecs::Tests::GetCreatedGroupIds().Contains(UE::Flecs::Tests::GroupByWithTagGroupId)));

		const uint64* WithoutTagGroupContext = static_cast<const uint64*>(
			Query.GetGroupContext(UE::Flecs::Tests::GroupByWithoutTagGroupId));
		ASSERT_THAT(IsNotNull(WithoutTagGroupContext));
		ASSERT_THAT(IsTrue(*WithoutTagGroupContext == UE::Flecs::Tests::GroupByWithoutTagGroupId));
		
		const uint64* WithTagGroupContext = static_cast<const uint64*>(
			Query.GetGroupContext(UE::Flecs::Tests::GroupByWithTagGroupId));
		ASSERT_THAT(IsNotNull(WithTagGroupContext));
		ASSERT_THAT(IsTrue(*WithTagGroupContext == UE::Flecs::Tests::GroupByWithTagGroupId));

		ASSERT_THAT(IsTrue(Query.Destroy()));

		ASSERT_THAT(IsTrue(UE::Flecs::Tests::GetDestroyedGroupIds().Contains(UE::Flecs::Tests::GroupByWithoutTagGroupId)));
		ASSERT_THAT(IsTrue(UE::Flecs::Tests::GetDestroyedGroupIds().Contains(UE::Flecs::Tests::GroupByWithTagGroupId)));
		ASSERT_THAT(IsTrue(UE::Flecs::Tests::GetDestroyedGroupContextIds().Contains(UE::Flecs::Tests::GroupByWithoutTagGroupId)));
		ASSERT_THAT(IsTrue(UE::Flecs::Tests::GetDestroyedGroupContextIds().Contains(UE::Flecs::Tests::GroupByWithTagGroupId)));
	}

}; // FlecsQueryGroupingTests

#endif // WITH_AUTOMATION_TESTS
