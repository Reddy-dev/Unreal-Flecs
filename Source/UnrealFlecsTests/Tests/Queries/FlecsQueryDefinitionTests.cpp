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

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS(FlecsQueryConstructionTests,
								   "UnrealFlecs.Queries.Construction.Terms",
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
	TEST_METHOD(DefaultConstruction)
	{
		FFlecsQueryDefinition QueryDefinition;

		ASSERT_THAT(AreEqual(QueryDefinition.CacheType, EFlecsQueryCacheType::Default));
		ASSERT_THAT(AreEqual(QueryDefinition.Flags, static_cast<uint8>(EFlecsQueryFlags::None)));
		ASSERT_THAT(AreEqual(QueryDefinition.Terms.Num(), 0));
		ASSERT_THAT(AreEqual(QueryDefinition.OtherExpressions.Num(), 0));
		
		flecs::query_builder<> QueryBuilder(World()->GetNativeFlecsWorld());
		FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal(QueryBuilder);
		QueryDefinition.Apply(World(), QueryBuilderView);
		flecs::query<> Query = QueryBuilder.build();
		
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
	}
	
	TEST_METHOD(Construction_WithScriptStructTagTerm_ScriptStructAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptStruct> InputTypeStruct;
		InputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = FFlecsTestStruct_Tag::StaticStruct();
		
		TermExpression1.Term.Input.First = InputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(World()->GetNativeFlecsWorld());
		FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal(QueryBuilder);
		QueryDefinition.Apply(World(), QueryBuilderView);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(Construction_WithScriptStructValueTerm_ScriptStructAPI)
	{
		static const FFlecsTestStruct_Value TestValue { 84 };
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptStruct> InputTypeStruct;
		InputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = FFlecsTestStruct_Value::StaticStruct();
		
		TermExpression1.Term.Input.First = InputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(World()->GetNativeFlecsWorld());
		FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal(QueryBuilder);
		QueryDefinition.Apply(World(), QueryBuilderView);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Set(FFlecsTestStruct_Value::StaticStruct(), &TestValue);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));
		
		const FFlecsTestStruct_Value& RetrievedValue = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(RetrievedValue.Value == 84));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		Query.each([&](flecs::iter& Iter, size_t Index)
		{
			const FFlecsTestStruct_Value& Value = Iter.field_at<FFlecsTestStruct_Value>(0, Index);
			ASSERT_THAT(AreEqual(Value.Value, 84));
		});
	}
	
	TEST_METHOD(Construction_WithScriptStructTagTerm_StringAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> InputTypeStruct;
		InputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("FFlecsTestStruct_Tag");
		
		TermExpression1.Term.Input.First = InputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(World()->GetNativeFlecsWorld());
		FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal(QueryBuilder);
		QueryDefinition.Apply(World(), QueryBuilderView);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(Construction_WithScriptStructValueTerm_StringAPI)
	{
		static const FFlecsTestStruct_Value TestValue { 84 };
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> InputTypeStruct;
		InputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("FFlecsTestStruct_Value");
		
		TermExpression1.Term.Input.First = InputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(World()->GetNativeFlecsWorld());
		FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal(QueryBuilder);
		QueryDefinition.Apply(World(), QueryBuilderView);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Set(FFlecsTestStruct_Value::StaticStruct(), &TestValue);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));
		
		const FFlecsTestStruct_Value& RetrievedValue = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(RetrievedValue.Value == 84));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		Query.each([&](flecs::iter& Iter, size_t Index)
		{
			const FFlecsTestStruct_Value& Value = Iter.field_at<FFlecsTestStruct_Value>(0, Index);
			ASSERT_THAT(AreEqual(Value.Value, 84));
		});
	}
	
}; // FlecsQueryConstructionTests

#endif // WITH_AUTOMATION_TESTS
