// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

#include "Entities/FlecsEntityRecord.h"
#include "Queries/FlecsQuery.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#include "Queries/FlecsQueryDefinition.h"
#include "Queries/FlecsQueryDefinitionRecordFragment.h"
#include "Queries/Generator/FlecsQueryGeneratorInputType.h"

// @TODO: add pair testing

/**
 * Layout of the tests:
 * A. Construction Tests
 * B. Query Definition Entity Record Fragment Builder
 **/
TEST_CLASS_WITH_FLAGS(B2_UnrealFlecsQueryDefinitionTests,
							   "UnrealFlecs.B2_QueryDefinition",
							   EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
								| EAutomationTestFlags::CriticalPriority)
{

	inline static TUniquePtr<FFlecsTestFixtureRAII> Fixture;
	inline static TObjectPtr<UFlecsWorld> FlecsWorld = nullptr;

	BEFORE_EACH()
	{
		Fixture = MakeUnique<FFlecsTestFixtureRAII>();
		FlecsWorld = Fixture->Fixture.GetFlecsWorld();
		
		FlecsWorld->RegisterComponentType<FFlecsTest_CPPStruct>();
		FlecsWorld->RegisterComponentType<FFlecsTest_CPPStructValue>();
		
		FlecsWorld->RegisterComponentType(FFlecsTestStruct_Value::StaticStruct());
		FlecsWorld->RegisterComponentType(FFlecsTestStruct_Tag::StaticStruct());
		FlecsWorld->RegisterComponentType(FFlecsTestStruct_PairIsTag::StaticStruct());
		FlecsWorld->RegisterComponentType(FUSTRUCTPairTestComponent::StaticStruct());
		FlecsWorld->RegisterComponentType(FUSTRUCTPairTestComponent_Second::StaticStruct());
		FlecsWorld->RegisterComponentType<FUSTRUCTPairTestComponent_Data>();
		
		FlecsWorld->RegisterComponentType<ETestEnum>();
		FlecsWorld->RegisterComponentType(StaticEnum<EFlecsTestEnum_UENUM>());
	}

	AFTER_EACH()
	{
		FlecsWorld = nullptr;
	}

	AFTER_ALL()
	{
		Fixture.Reset();
	}
	
	TEST_METHOD(A1_DefaultConstruction)
	{
		FFlecsQueryDefinition QueryDefinition;

		ASSERT_THAT(AreEqual(QueryDefinition.CacheType, EFlecsQueryCacheType::Default));
		ASSERT_THAT(IsFalse(QueryDefinition.bDetectChanges));
		ASSERT_THAT(AreEqual(QueryDefinition.Flags, static_cast<uint8>(EFlecsQueryFlags::None)));
		ASSERT_THAT(AreEqual(QueryDefinition.Terms.Num(), 0));
		ASSERT_THAT(AreEqual(QueryDefinition.OtherExpressions.Num(), 0));
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
	}
	
	TEST_METHOD(A2_Construction_WithScriptStructTagTerm_ScriptStructAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptStruct> InputTypeStruct;
		InputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = FFlecsTestStruct_Tag::StaticStruct();
		
		TermExpression1.Term.InputType = InputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(A3_Construction_WithScriptStructValueTerm_ScriptStructAPI)
	{
		static const FFlecsTestStruct_Value TestValue { 84 };
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptStruct> InputTypeStruct;
		InputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = FFlecsTestStruct_Value::StaticStruct();
		
		TermExpression1.Term.InputType = InputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
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
	
	TEST_METHOD(A4_Construction_WithScriptStructTagTerm_StringAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> InputTypeStruct;
		InputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("FFlecsTestStruct_Tag");
		
		TermExpression1.Term.InputType = InputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(A5_Construction_WithScriptStructValueTerm_StringAPI)
	{
		static const FFlecsTestStruct_Value TestValue { 84 };
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> InputTypeStruct;
		InputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("FFlecsTestStruct_Value");
		
		TermExpression1.Term.InputType = InputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
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
	
	TEST_METHOD(A6_Construction_WithScriptStructPairTerms_ScriptStructAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_Pair> InputTypeStruct;
		InputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptStruct> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = FUSTRUCTPairTestComponent::StaticStruct();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptStruct> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = FUSTRUCTPairTestComponent_Second::StaticStruct();
		
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTypeStruct;
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondTypeStruct;
		
		TermExpression1.Term.InputType = InputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.AddPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(A7_Construction_WithScriptStructPairTerms_StringAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_Pair> InputTypeStruct;
		InputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("FUSTRUCTPairTestComponent");
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("FUSTRUCTPairTestComponent_Second");
		
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTypeStruct;
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondTypeStruct;
		
		TermExpression1.Term.InputType = InputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.AddPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(A8_Construction_WithScriptStructPairTerms_ScriptStructAPI_StringAPI_Combined)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_Pair> InputTypeStruct;
		InputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptStruct> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = FUSTRUCTPairTestComponent::StaticStruct();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("FUSTRUCTPairTestComponent_Second");
		
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTypeStruct;
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondTypeStruct;
		
		TermExpression1.Term.InputType = InputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.AddPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(A9_Construction_WithScriptStructPairTermAndWildcard_ScriptStructAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_Pair> InputTypeStruct;
		InputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptStruct> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = FUSTRUCTPairTestComponent::StaticStruct();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_FlecsId> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = flecs::Wildcard;
		
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTypeStruct;
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondTypeStruct;
		
		TermExpression1.Term.InputType = InputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.AddPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = FlecsWorld->CreateEntity()
			.AddPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Data>();
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Data>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 2));
	}
		
	TEST_METHOD(A10_Construction_WithScriptEnumPairTerm_CPPAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_Pair> InputTypeStruct;
		InputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptEnum> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = StaticEnum<EFlecsTestEnum_UENUM>();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptEnumConstant> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = FSolidEnumSelector::Make<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One);
		
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTypeStruct;
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondTypeStruct;
		
		TermExpression1.Term.InputType = InputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = FlecsWorld->CreateEntity()
			.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Two);
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(A11_Construction_WithScriptEnumPairTermAndWildcard_CPPAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_Pair> InputTypeStruct;
		InputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptEnum> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = StaticEnum<EFlecsTestEnum_UENUM>();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_FlecsId> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = flecs::Wildcard;
		
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTypeStruct;
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondTypeStruct;
		
		TermExpression1.Term.InputType = InputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = FlecsWorld->CreateEntity()
			.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Two);
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 2));
	}
	
	TEST_METHOD(A12_Construction_WithScriptEnumPairTerm_StringAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_Pair> InputTypeStruct;
		InputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("EFlecsTestEnum_UENUM");
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("EFlecsTestEnum_UENUM.One");
		
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTypeStruct;
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondTypeStruct;
		
		TermExpression1.Term.InputType = InputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = FlecsWorld->CreateEntity()
			.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Two);
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(A13_Construction_WithScriptEnumPairTermAndWildcard_StringAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_Pair> InputTypeStruct;
		InputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("EFlecsTestEnum_UENUM");
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_FlecsId> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = flecs::Wildcard;
		
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTypeStruct;
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondTypeStruct;
		
		TermExpression1.Term.InputType = InputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = FlecsWorld->CreateEntity()
			.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Two);
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 2));
	}
	
	TEST_METHOD(A14_Construction_WithCPPEnumPairTerm_StringAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_Pair> InputTypeStruct;
		InputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("ETestEnum");
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("ETestEnum.One");
		
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTypeStruct;
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondTypeStruct;
		
		TermExpression1.Term.InputType = InputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add<ETestEnum>(ETestEnum::One);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<ETestEnum>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = FlecsWorld->CreateEntity()
			.Add<ETestEnum>(ETestEnum::Two);
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<ETestEnum>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(A15_Construction_WithCPPEnumPairTermAndWildcard_StringAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_Pair> InputTypeStruct;
		InputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_String> FirstTypeStruct;
		FirstTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		FirstTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = TEXT("ETestEnum");
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_FlecsId> SecondTypeStruct;
		SecondTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		SecondTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = flecs::Wildcard;
		
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTypeStruct;
		InputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondTypeStruct;
		
		TermExpression1.Term.InputType = InputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add<ETestEnum>(ETestEnum::One);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<ETestEnum>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = FlecsWorld->CreateEntity()
			.Add<ETestEnum>(ETestEnum::Two);
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<ETestEnum>(flecs::Wildcard)));
		
		ASSERT_THAT(IsTrue(Query.count() == 2));
	}
	
	TEST_METHOD(A16_Construction_WithScriptStructTermAndWithoutTag_ScriptStructAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptStruct> WithoutInputTypeStruct;
		WithoutInputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		WithoutInputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = FFlecsTestStruct_Tag::StaticStruct();
		
		TermExpression1.Term.InputType = WithoutInputTypeStruct;
		TermExpression1.Operator = EFlecsQueryOperator::Not;
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptStruct> WithInputTypeStruct;
		WithInputTypeStruct.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		WithInputTypeStruct.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = FFlecsTestStruct_Value::StaticStruct();
		
		FFlecsQueryTermExpression TermExpression2;
		TermExpression2.Term.InputType = WithInputTypeStruct;
		
		QueryDefinition.Terms.Add(TermExpression2);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		static const FFlecsTestStruct_Value TestValue { 256 };
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Set(FFlecsTestStruct_Value::StaticStruct(), &TestValue);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));
		
		const FFlecsTestStruct_Value& RetrievedValue = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(RetrievedValue.Value == 256));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = FlecsWorld->CreateEntity()
			.Add<FFlecsTestStruct_Tag>()
			.Set(FFlecsTestStruct_Value::StaticStruct(), &TestValue);
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity2.Has(FFlecsTestStruct_Value::StaticStruct())));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(B1_Construction_WithNoTermsOrOtherExpressions_BuildsQuerySuccessfully)
	{
		FFlecsEntityRecord QueryEntityRecord = FFlecsEntityRecord().Builder()
			.FragmentScope<FFlecsQueryDefinitionRecordFragment>()
			.End()
			.Build();
		
		ASSERT_THAT(IsTrue(QueryEntityRecord.HasFragment<FFlecsQueryDefinitionRecordFragment>()));
		
		FFlecsEntityHandle QueryEntityHandle = FlecsWorld->CreateEntityWithRecord(QueryEntityRecord);
		ASSERT_THAT(IsTrue(QueryEntityHandle.IsValid()));
		
		FFlecsQuery Query = FlecsWorld->GetQueryFromEntity(QueryEntityHandle);
		ASSERT_THAT(IsTrue(Query));
		
		ASSERT_THAT(IsTrue(Query.GetCount() == 0));
	}
	
	TEST_METHOD(B2_Construction_WithTagTermScriptStruct_CPPAPI)
	{
		FFlecsEntityRecord QueryEntityRecord = FFlecsEntityRecord().Builder()
			.FragmentScope<FFlecsQueryDefinitionRecordFragment>()
				.With<FFlecsTestStruct_Tag>() // 0
			.End()
			.Build();
		
		ASSERT_THAT(IsTrue(QueryEntityRecord.HasFragment<FFlecsQueryDefinitionRecordFragment>()));
		
		FFlecsEntityHandle QueryEntityHandle = FlecsWorld->CreateEntityWithRecord(QueryEntityRecord);
		ASSERT_THAT(IsTrue(QueryEntityHandle.IsValid()));
		
		FFlecsQuery Query = FlecsWorld->GetQueryFromEntity(QueryEntityHandle);
		ASSERT_THAT(IsTrue(Query));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		
		ASSERT_THAT(IsTrue(Query.GetCount() == 1));
	}
	
	TEST_METHOD(B3_Construction_WithValueTermScriptStruct_ScriptStructAPI)
	{
		static const FFlecsTestStruct_Value TestValue { 168 };
		FFlecsEntityRecord QueryEntityRecord = FFlecsEntityRecord().Builder()
			.FragmentScope<FFlecsQueryDefinitionRecordFragment>()
				.With(FFlecsTestStruct_Value::StaticStruct()) // 0
			.End()
			.Build();
		
		ASSERT_THAT(IsTrue(QueryEntityRecord.HasFragment<FFlecsQueryDefinitionRecordFragment>()));
		
		FFlecsEntityHandle QueryEntityHandle = FlecsWorld->CreateEntityWithRecord(QueryEntityRecord);
		ASSERT_THAT(IsTrue(QueryEntityHandle.IsValid()));
		
		FFlecsQuery Query = FlecsWorld->GetQueryFromEntity(QueryEntityHandle);
		ASSERT_THAT(IsTrue(Query));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Set(FFlecsTestStruct_Value::StaticStruct(), &TestValue);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));
		
		const FFlecsTestStruct_Value& RetrievedValue = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(RetrievedValue.Value == 168));
		
		ASSERT_THAT(IsTrue(Query.GetCount() == 1));
		
		Query.Get().each([&](flecs::iter& Iter, size_t Index)
		{
			const FFlecsTestStruct_Value& Value = Iter.field_at<FFlecsTestStruct_Value>(0, Index);
			ASSERT_THAT(AreEqual(Value.Value, 168));
		});
	}
	
	TEST_METHOD(B4_Construction_WithPairTermScriptStructs_CPPAPI)
	{
		FFlecsEntityRecord QueryEntityRecord = FFlecsEntityRecord().Builder()
			.FragmentScope<FFlecsQueryDefinitionRecordFragment>()
				.WithPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>() // 0
			.End()
			.Build();
		
		ASSERT_THAT(IsTrue(QueryEntityRecord.HasFragment<FFlecsQueryDefinitionRecordFragment>()));
		
		FFlecsEntityHandle QueryEntityHandle = FlecsWorld->CreateEntityWithRecord(QueryEntityRecord);
		ASSERT_THAT(IsTrue(QueryEntityHandle.IsValid()));
		
		FFlecsQuery Query = FlecsWorld->GetQueryFromEntity(QueryEntityHandle);
		ASSERT_THAT(IsTrue(Query));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.AddPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
		
		ASSERT_THAT(IsTrue(Query.GetCount() == 1));
	}
	
	TEST_METHOD(B5_Construction_WithPairTermScriptStructAndWildcard_CPPAPI)
	{
		FFlecsEntityRecord QueryEntityRecord = FFlecsEntityRecord().Builder()
			.FragmentScope<FFlecsQueryDefinitionRecordFragment>()
				.WithPair<FUSTRUCTPairTestComponent>(flecs::Wildcard) // 0
			.End()
			.Build();
		
		ASSERT_THAT(IsTrue(QueryEntityRecord.HasFragment<FFlecsQueryDefinitionRecordFragment>()));
		
		FFlecsEntityHandle QueryEntityHandle = FlecsWorld->CreateEntityWithRecord(QueryEntityRecord);
		ASSERT_THAT(IsTrue(QueryEntityHandle.IsValid()));
		
		FFlecsQuery Query = FlecsWorld->GetQueryFromEntity(QueryEntityHandle);
		ASSERT_THAT(IsTrue(Query));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.AddPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
		
		ASSERT_THAT(IsTrue(Query.GetCount() == 1));
		
		FFlecsEntityHandle TestEntity2 = FlecsWorld->CreateEntity()
			.AddPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Data>();
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Data>()));
		
		ASSERT_THAT(IsTrue(Query.GetCount() == 2));
	}
	
	TEST_METHOD(B6_Construction_WithPairTermScriptStructs_CPPAPI_ScriptStructAPI)
	{
		FFlecsEntityRecord QueryEntityRecord = FFlecsEntityRecord().Builder()
			.FragmentScope<FFlecsQueryDefinitionRecordFragment>()
				.WithPair<FUSTRUCTPairTestComponent>(FUSTRUCTPairTestComponent_Second::StaticStruct()) // 0
			.End()
			.Build();
		
		ASSERT_THAT(IsTrue(QueryEntityRecord.HasFragment<FFlecsQueryDefinitionRecordFragment>()));
		
		FFlecsEntityHandle QueryEntityHandle = FlecsWorld->CreateEntityWithRecord(QueryEntityRecord);
		ASSERT_THAT(IsTrue(QueryEntityHandle.IsValid()));
		
		FFlecsQuery Query = FlecsWorld->GetQueryFromEntity(QueryEntityHandle);
		ASSERT_THAT(IsTrue(Query));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.AddPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
		
		ASSERT_THAT(IsTrue(Query.GetCount() == 1));
	}
	
	TEST_METHOD(B7_Construction_WithPairTermScriptStructs_ScriptStructAPI_CPPAPI)
	{
		FFlecsEntityRecord QueryEntityRecord = FFlecsEntityRecord().Builder()
			.FragmentScope<FFlecsQueryDefinitionRecordFragment>()
				.WithPairSecond<FUSTRUCTPairTestComponent_Second>(FUSTRUCTPairTestComponent::StaticStruct()) // 0
			.End()
			.Build();
		
		ASSERT_THAT(IsTrue(QueryEntityRecord.HasFragment<FFlecsQueryDefinitionRecordFragment>()));
		
		FFlecsEntityHandle QueryEntityHandle = FlecsWorld->CreateEntityWithRecord(QueryEntityRecord);
		ASSERT_THAT(IsTrue(QueryEntityHandle.IsValid()));
		
		FFlecsQuery Query = FlecsWorld->GetQueryFromEntity(QueryEntityHandle);
		ASSERT_THAT(IsTrue(Query));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.AddPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
		
		ASSERT_THAT(IsTrue(Query.GetCount() == 1));
	}
	

}; // End of B2_UnrealFlecsQueryDefinitionTests

#endif // WITH_AUTOMATION_TESTS
