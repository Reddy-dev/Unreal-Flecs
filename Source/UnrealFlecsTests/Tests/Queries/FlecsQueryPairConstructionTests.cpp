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

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS(FlecsQueryPairConstructionTests,
								   "UnrealFlecs.Queries.Construction.Pairs",
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
	TEST_METHOD(Construction_WithScriptStructPairTerms_ScriptStructAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptStruct> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = FUSTRUCTPairTestComponent::StaticStruct();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptStruct> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = FUSTRUCTPairTestComponent_Second::StaticStruct();
		
		TermExpression1.Term.Input.bPair = true;
		TermExpression1.Term.Input.First = FirstTypeStruct;
		TermExpression1.Term.Input.Second = SecondTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(World()->GetNativeFlecsWorld());
		FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal(QueryBuilder);
		QueryDefinition.Apply(World(), QueryBuilderView);
		
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.AddPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(Construction_WithScriptStructPairTerms_StringAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("FUSTRUCTPairTestComponent");
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("FUSTRUCTPairTestComponent_Second");
		
		TermExpression1.Term.Input.bPair = true;
		TermExpression1.Term.Input.First = FirstTypeStruct;
		TermExpression1.Term.Input.Second = SecondTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(World()->GetNativeFlecsWorld());
		FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal(QueryBuilder);
		QueryDefinition.Apply(World(), QueryBuilderView);
		
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.AddPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(Construction_WithScriptStructPairTerms_ScriptStructAPI_StringAPI_Combined)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptStruct> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = FUSTRUCTPairTestComponent::StaticStruct();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("FUSTRUCTPairTestComponent_Second");
		
		TermExpression1.Term.Input.bPair = true;
		TermExpression1.Term.Input.First = FirstTypeStruct;
		TermExpression1.Term.Input.Second = SecondTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(World()->GetNativeFlecsWorld());
		FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal(QueryBuilder);
		QueryDefinition.Apply(World(), QueryBuilderView);
		
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.AddPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(Construction_WithScriptStructPairTermAndWildcard_ScriptStructAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptStruct> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = FUSTRUCTPairTestComponent::StaticStruct();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_FlecsId> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = flecs::Wildcard;
		
		TermExpression1.Term.Input.bPair = true;
		TermExpression1.Term.Input.First = FirstTypeStruct;
		TermExpression1.Term.Input.Second = SecondTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(World()->GetNativeFlecsWorld());
		FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal(QueryBuilder);
		QueryDefinition.Apply(World(), QueryBuilderView);
		
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.AddPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = World()->CreateEntity()
			.AddPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Data>();
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Data>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 2));
	}
		
	TEST_METHOD(Construction_WithScriptEnumPairTerm_CPPAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptEnum> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = StaticEnum<EFlecsTestEnum_UENUM>();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptEnumConstant> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = FSolidEnumSelector::Make<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One);
		
		TermExpression1.Term.Input.bPair = true;
		TermExpression1.Term.Input.First = FirstTypeStruct;
		TermExpression1.Term.Input.Second = SecondTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(World()->GetNativeFlecsWorld());
		FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal(QueryBuilder);
		QueryDefinition.Apply(World(), QueryBuilderView);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = World()->CreateEntity()
			.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Two);
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(Construction_WithScriptEnumPairTermAndWildcard_CPPAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptEnum> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = StaticEnum<EFlecsTestEnum_UENUM>();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_FlecsId> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = flecs::Wildcard;
		
		TermExpression1.Term.Input.bPair = true;
		TermExpression1.Term.Input.First = FirstTypeStruct;
		TermExpression1.Term.Input.Second = SecondTypeStruct;

		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(World()->GetNativeFlecsWorld());
		FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal(QueryBuilder);
		QueryDefinition.Apply(World(), QueryBuilderView);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = World()->CreateEntity()
			.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Two);
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 2));
	}
	
	TEST_METHOD(Construction_WithScriptEnumPairTerm_StringAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("EFlecsTestEnum_UENUM");
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("EFlecsTestEnum_UENUM.One");
		
		TermExpression1.Term.Input.bPair = true;
		TermExpression1.Term.Input.First = FirstTypeStruct;
		TermExpression1.Term.Input.Second = SecondTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(World()->GetNativeFlecsWorld());
		FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal(QueryBuilder);
		QueryDefinition.Apply(World(), QueryBuilderView);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = World()->CreateEntity()
			.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Two);
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(Construction_WithScriptEnumPairTermAndWildcard_StringAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("EFlecsTestEnum_UENUM");
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_FlecsId> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = flecs::Wildcard;
		
		TermExpression1.Term.Input.bPair = true;
		TermExpression1.Term.Input.First = FirstTypeStruct;
		TermExpression1.Term.Input.Second = SecondTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(World()->GetNativeFlecsWorld());
		FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal(QueryBuilder);
		QueryDefinition.Apply(World(), QueryBuilderView);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = World()->CreateEntity()
			.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Two);
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 2));
	}
	
	TEST_METHOD(Construction_WithCPPEnumPairTerm_StringAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("ETestEnum");
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("ETestEnum.One");
		
		TermExpression1.Term.Input.bPair = true;
		TermExpression1.Term.Input.First = FirstTypeStruct;
		TermExpression1.Term.Input.Second = SecondTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(World()->GetNativeFlecsWorld());
		FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal(QueryBuilder);
		QueryDefinition.Apply(World(), QueryBuilderView);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Add<ETestEnum>(ETestEnum::One);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<ETestEnum>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = World()->CreateEntity()
			.Add<ETestEnum>(ETestEnum::Two);
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<ETestEnum>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(Construction_WithCPPEnumPairTermAndWildcard_StringAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("ETestEnum");
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_FlecsId> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = flecs::Wildcard;
		
		TermExpression1.Term.Input.bPair = true;
		TermExpression1.Term.Input.First = FirstTypeStruct;
		TermExpression1.Term.Input.Second = SecondTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(World()->GetNativeFlecsWorld());
		FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal(QueryBuilder);
		QueryDefinition.Apply(World(), QueryBuilderView);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Add<ETestEnum>(ETestEnum::One);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<ETestEnum>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = World()->CreateEntity()
			.Add<ETestEnum>(ETestEnum::Two);
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<ETestEnum>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 2));
	}
	
	TEST_METHOD(Construction_WithScriptStructTermAndWithoutTag_ScriptStructAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptStruct> WithoutInputTypeStruct;
		WithoutInputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		WithoutInputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = FFlecsTestStruct_Tag::StaticStruct();
		
		TermExpression1.Term.Input.First = WithoutInputTypeStruct;
		TermExpression1.Operator = EFlecsQueryOperator::Not;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptStruct> WithInputTypeStruct;
		WithInputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		WithInputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = FFlecsTestStruct_Value::StaticStruct();
		
		FFlecsQueryTermExpression TermExpression2;
		TermExpression2.Term.Input.First = WithInputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression2);
		
		flecs::query_builder<> QueryBuilder(World()->GetNativeFlecsWorld());
		FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal(QueryBuilder);
		QueryDefinition.Apply(World(), QueryBuilderView);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		static const FFlecsTestStruct_Value TestValue { 256 };
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Set(FFlecsTestStruct_Value::StaticStruct(), &TestValue);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));
		
		const FFlecsTestStruct_Value& RetrievedValue = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(RetrievedValue.Value == 256));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = World()->CreateEntity()
			.Add<FFlecsTestStruct_Tag>()
			.Set(FFlecsTestStruct_Value::StaticStruct(), &TestValue);
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity2.Has(FFlecsTestStruct_Value::StaticStruct())));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
}; // FlecsQueryPairConstructionTests

#endif // WITH_AUTOMATION_TESTS
