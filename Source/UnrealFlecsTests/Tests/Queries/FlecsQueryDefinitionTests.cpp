// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "FlecsQueryDefinitionTestTypes.h"
#include "Queries/FlecsQuery.h"

#include "Queries/FlecsQueryDefinition.h"
#include "Queries/Generator/FlecsQueryGeneratorInputType.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

// @TODO: add pair testing

/**
 * Layout of the tests:
 * A. Construction Tests
 * B. Query Builder 
 * C. Query Order By Tests
 * D. Query Group By Tests
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
	
	TEST_METHOD(B1_BuilderConstruction_WithScriptStructTagTerm_ScriptStructAPI)
	{
		FFlecsQuery Query = FlecsWorld->CreateQueryBuilder()
			.With(FFlecsTestStruct_Tag::StaticStruct())
			.Build();
		
		ASSERT_THAT(IsTrue(Query.IsValid()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = FlecsWorld->CreateEntity()
			.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<FFlecsTestStruct_Tag>()));
			
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 2));
		
		TestEntity2.Remove<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		TestEntity.Remove<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsFalse(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 0));
	}
	
	TEST_METHOD(B2_BuilderConstruction_WithScriptStructTagTerm_CPPAPI)
	{
		FFlecsQuery Query = FlecsWorld->CreateQueryBuilder()
			.With<FFlecsTestStruct_Tag>()
			.Build();
		
		ASSERT_THAT(IsTrue(Query.IsValid()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = FlecsWorld->CreateEntity()
			.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<FFlecsTestStruct_Tag>()));
			
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 2));
		
		TestEntity2.Remove<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		TestEntity.Remove<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsFalse(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 0));
	}
	
	TEST_METHOD(B3_BuilderConstruction_WithScriptStructTagTerm_StringAPI)
	{
		FFlecsQuery Query = FlecsWorld->CreateQueryBuilder()
			.With("FFlecsTestStruct_Tag")
			.Build();
		
		ASSERT_THAT(IsTrue(Query.IsValid()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = FlecsWorld->CreateEntity()
			.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<FFlecsTestStruct_Tag>()));
			
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 2));
		
		TestEntity2.Remove<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		TestEntity.Remove<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsFalse(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 0));
	}
	
	TEST_METHOD(B4_BuilderConstruction_WithScriptStructTagTermAndWithoutTag_ScriptStructAPI)
	{
		FFlecsQuery Query = FlecsWorld->CreateQueryBuilder()
			.Without(FFlecsTestStruct_Tag::StaticStruct())
			.With(FFlecsTestStruct_Value::StaticStruct())
			.Build();
		
		ASSERT_THAT(IsTrue(Query.IsValid()));
		
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
	
	TEST_METHOD(B5_BuilderConstruction_WithCPPTagTermAndWithoutTag_CPPAPI)
	{
		FFlecsQuery Query = FlecsWorld->CreateQueryBuilder()
			.Without<FFlecsTestStruct_Tag>()
			.With<FFlecsTest_CPPStructValue>()
			.Build();
		
		ASSERT_THAT(IsTrue(Query.IsValid()));
		
		static const FFlecsTest_CPPStructValue TestValue { 256 };
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Set<FFlecsTest_CPPStructValue>(TestValue);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTest_CPPStructValue>()));
		
		const FFlecsTest_CPPStructValue& RetrievedValue = TestEntity.Get<FFlecsTest_CPPStructValue>();
		ASSERT_THAT(IsTrue(RetrievedValue.Value == 256));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = FlecsWorld->CreateEntity()
			.Add<FFlecsTestStruct_Tag>()
			.Set<FFlecsTest_CPPStructValue>(TestValue);
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<FFlecsTest_CPPStructValue>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(B6_BuilderConstruction_WithTypedQueryDefinition_ScriptStructTagTerm_ScriptStructAPI)
	{
		TTypedFlecsQuery<FFlecsTestStruct_Tag> Query = FlecsWorld->CreateQueryBuilder<FFlecsTestStruct_Tag>()
			.Build();
		
		ASSERT_THAT(IsTrue(Query.IsValid()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = FlecsWorld->CreateEntity()
			.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<FFlecsTestStruct_Tag>()));
			
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 2));
		
		TestEntity2.Remove<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		TestEntity.Remove<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsFalse(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 0));
	}
	
	TEST_METHOD(B7_BuilderConstruction_WithTypedQueryDefinition_ScriptStructTagTermAndScriptStructValueTerm_ScriptStructAPI)
	{
		TTypedFlecsQuery<FFlecsTestStruct_Value> Query = FlecsWorld->CreateQueryBuilder<const FFlecsTestStruct_Value>()
			.With<FFlecsTestStruct_Tag>()
			.Build();
		
		ASSERT_THAT(IsTrue(Query.IsValid()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add<FFlecsTestStruct_Tag>()
			.Set<FFlecsTestStruct_Value>({ 512 });
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = FlecsWorld->CreateEntity()
			.Add<FFlecsTestStruct_Tag>()
			.Set<FFlecsTestStruct_Value>({ 256 });
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<FFlecsTestStruct_Tag>()));
			
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 2));
		
		int32 FoundValueSum = 0;
		Query.each([&](flecs::iter& Iter, size_t Index, const FFlecsTestStruct_Value& Value)
		{
			FoundValueSum += Value.Value;
		});
		
		ASSERT_THAT(IsTrue(FoundValueSum == (512 + 256)));
		
		TestEntity2.Remove<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		TestEntity.Remove<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsFalse(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 0));
	}
	
	TEST_METHOD(C1_BuilderAPI_OrderByCallbackDefinition_ScriptStructValue_Ascending_ScriptStructAPI)
	{
		FFlecsQuery Query = FlecsWorld->CreateQueryBuilder()
			.With(FFlecsTestStruct_Value::StaticStruct())
			.OrderByCallbackDefinition(FFlecsTestStruct_Value::StaticStruct(), 
				TInstancedStruct<FFlecsQueryOrderByCallbackDefinitionTest_ScriptStruct>::Make(EFlecsTestQueryOrderByFunction::Ascending))
			.Build();
		
		{
			static const FFlecsTestStruct_Value V3{ 3 };
			static const FFlecsTestStruct_Value V1{ 1 };
			static const FFlecsTestStruct_Value V2{ 2 };

			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V1);
			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V2);
			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V3);
		}
		
		TArray<int32> ExpectedOrder = { 1, 2, 3 };
		int32 Index = 0;
		Query.each([&](flecs::iter& Iter, size_t IndexInIter)
		{
			const FFlecsTestStruct_Value& Value = Iter.field_at<const FFlecsTestStruct_Value>(0, IndexInIter);
			ASSERT_THAT(IsTrue(Value.Value == ExpectedOrder[Index]));
			Index++;
		});
	}
	
	TEST_METHOD(C2_BuilderAPI_OrderByCallbackDefinition_ScriptStructValue_Descending_ScriptStructAPI)
	{
		FFlecsQuery Query = FlecsWorld->CreateQueryBuilder()
			.With(FFlecsTestStruct_Value::StaticStruct())
			.OrderByCallbackDefinition(FFlecsTestStruct_Value::StaticStruct(), 
				TInstancedStruct<FFlecsQueryOrderByCallbackDefinitionTest_ScriptStruct>::Make(EFlecsTestQueryOrderByFunction::Descending))
			.Build();
		
		{
			static const FFlecsTestStruct_Value V3{ 3 };
			static const FFlecsTestStruct_Value V1{ 1 };
			static const FFlecsTestStruct_Value V2{ 2 };

			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V1);
			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V2);
			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V3);
		}
		
		TArray<int32> ExpectedOrder = { 3, 2, 1 };
		int32 Index = 0;
		Query.each([&](flecs::iter& Iter, size_t IndexInIter)
		{
			const FFlecsTestStruct_Value& Value = Iter.field_at<const FFlecsTestStruct_Value>(0, IndexInIter);
			ASSERT_THAT(IsTrue(Value.Value == ExpectedOrder[Index]));
			Index++;
		});
	}
	
	TEST_METHOD(C3_BuilderAPI_OrderByCallbackDefinition_ScriptStructValue_Ascending_CPPAPI)
	{
		FFlecsQuery Query = FlecsWorld->CreateQueryBuilder()
			.With(FFlecsTestStruct_Value::StaticStruct())
			.OrderByCallbackDefinition<FFlecsTestStruct_Value>(
				TInstancedStruct<FFlecsQueryOrderByCallbackDefinitionTest_ScriptStruct>::Make(EFlecsTestQueryOrderByFunction::Ascending))
			.Build();
		
		{
			static const FFlecsTestStruct_Value V3{ 3 };
			static const FFlecsTestStruct_Value V1{ 1 };
			static const FFlecsTestStruct_Value V2{ 2 };

			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V1);
			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V2);
			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V3);
		}
		
		TArray<int32> ExpectedOrder = { 1, 2, 3 };
		int32 Index = 0;
		Query.each([&](flecs::iter& Iter, size_t IndexInIter)
		{
			const FFlecsTestStruct_Value& Value = Iter.field_at<const FFlecsTestStruct_Value>(0, IndexInIter);
			ASSERT_THAT(IsTrue(Value.Value == ExpectedOrder[Index]));
			Index++;
		});
	}
	
	TEST_METHOD(C4_BuilderAPI_OrderByCallbackDefinition_ScriptStructValue_Descending_CPPAPI)
	{
		FFlecsQuery Query = FlecsWorld->CreateQueryBuilder()
			.With(FFlecsTestStruct_Value::StaticStruct())
			.OrderByCallbackDefinition<FFlecsTestStruct_Value>(
				TInstancedStruct<FFlecsQueryOrderByCallbackDefinitionTest_ScriptStruct>::Make(EFlecsTestQueryOrderByFunction::Descending))
			.Build();
		
		{
			static const FFlecsTestStruct_Value V3{ 3 };
			static const FFlecsTestStruct_Value V1{ 1 };
			static const FFlecsTestStruct_Value V2{ 2 };

			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V1);
			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V2);
			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V3);
		}
		
		TArray<int32> ExpectedOrder = { 3, 2, 1 };
		int32 Index = 0;
		Query.each([&](flecs::iter& Iter, size_t IndexInIter)
		{
			const FFlecsTestStruct_Value& Value = Iter.field_at<const FFlecsTestStruct_Value>(0, IndexInIter);
			ASSERT_THAT(IsTrue(Value.Value == ExpectedOrder[Index]));
			Index++;
		});
	}
	
	TEST_METHOD(C5_BuilderAPI_OrderByOrderByCallbackDefinition_CppOnlyStructValue_Ascending_CPPAPI)
	{
		FFlecsQuery Query = FlecsWorld->CreateQueryBuilder()
			.With<FFlecsTest_CPPStructValue>()
			.OrderByCallbackDefinition<FFlecsTest_CPPStructValue>(
				TInstancedStruct<FFlecsQueryOrderByCallbackDefinitionTest_CPPType>::Make(EFlecsTestQueryOrderByFunction::Ascending))
			.Build();
		
		{
			static const FFlecsTest_CPPStructValue V3{ 3 };
			static const FFlecsTest_CPPStructValue V1{ 1 };
			static const FFlecsTest_CPPStructValue V2{ 2 };

			FlecsWorld->CreateEntity().Set<FFlecsTest_CPPStructValue>(V1);
			FlecsWorld->CreateEntity().Set<FFlecsTest_CPPStructValue>(V2);
			FlecsWorld->CreateEntity().Set<FFlecsTest_CPPStructValue>(V3);
		}
		
		TArray<int32> ExpectedOrder = { 1, 2, 3 };
		int32 Index = 0;
		Query.each([&](flecs::iter& Iter, size_t IndexInIter)
		{
			const FFlecsTest_CPPStructValue& Value = Iter.field_at<const FFlecsTest_CPPStructValue>(0, IndexInIter);
			ASSERT_THAT(IsTrue(Value.Value == ExpectedOrder[Index]));
			Index++;
		});
	}
	
	TEST_METHOD(C6_BuilderAPI_OrderByOrderByCallbackDefinition_CppOnlyStructValue_Descending_CPPAPI)
	{
		FFlecsQuery Query = FlecsWorld->CreateQueryBuilder()
			.With<FFlecsTest_CPPStructValue>()
			.OrderByCallbackDefinition<FFlecsTest_CPPStructValue>(
				TInstancedStruct<FFlecsQueryOrderByCallbackDefinitionTest_CPPType>::Make(EFlecsTestQueryOrderByFunction::Descending))
			.Build();
		
		{
			static const FFlecsTest_CPPStructValue V3{ 3 };
			static const FFlecsTest_CPPStructValue V1{ 1 };
			static const FFlecsTest_CPPStructValue V2{ 2 };

			FlecsWorld->CreateEntity().Set<FFlecsTest_CPPStructValue>(V1);
			FlecsWorld->CreateEntity().Set<FFlecsTest_CPPStructValue>(V2);
			FlecsWorld->CreateEntity().Set<FFlecsTest_CPPStructValue>(V3);
		}
		
		TArray<int32> ExpectedOrder = { 3, 2, 1 };
		int32 Index = 0;
		Query.each([&](flecs::iter& Iter, size_t IndexInIter)
		{
			const FFlecsTest_CPPStructValue& Value = Iter.field_at<const FFlecsTest_CPPStructValue>(0, IndexInIter);
			ASSERT_THAT(IsTrue(Value.Value == ExpectedOrder[Index]));
			Index++;
		});
	}
	
	TEST_METHOD(C7_BuilderAPI_OrderBy_ScriptStructValue_Ascending_StringAPI)
	{
		FFlecsQuery Query = FlecsWorld->CreateQueryBuilder()
			.With("FFlecsTestStruct_Value")
			.OrderBy(
				FFlecsTestStruct_Value::StaticStruct(),
				[](const FFlecsId IdA, const void* A, const FFlecsId IdB, const void* B) -> int32
			{
				const FFlecsTestStruct_Value* StructA = static_cast<const FFlecsTestStruct_Value*>(A);
				const FFlecsTestStruct_Value* StructB = static_cast<const FFlecsTestStruct_Value*>(B);
				return StructA->Value - StructB->Value;
			})
			.Build();
		
		{
			static const FFlecsTestStruct_Value V3{ 3 };
			static const FFlecsTestStruct_Value V1{ 1 };
			static const FFlecsTestStruct_Value V2{ 2 };

			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V1);
			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V2);
			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V3);
		}
		
		TArray<int32> ExpectedOrder = { 1, 2, 3 };
		int32 Index = 0;
		Query.each([&](flecs::iter& Iter, size_t IndexInIter)
		{
			const FFlecsTestStruct_Value& Value = Iter.field_at<const FFlecsTestStruct_Value>(0, IndexInIter);
			ASSERT_THAT(IsTrue(Value.Value == ExpectedOrder[Index]));
			Index++;
		});
	}
	
	TEST_METHOD(C8_BuilderAPI_OrderBy_ScriptStructValue_Descending_StringAPI)
	{
		FFlecsQuery Query = FlecsWorld->CreateQueryBuilder()
			.With("FFlecsTestStruct_Value")
			.OrderBy(
				FFlecsTestStruct_Value::StaticStruct(),
				[](const FFlecsId IdA, const void* A, const FFlecsId IdB, const void* B) -> int32
			{
				const FFlecsTestStruct_Value* StructA = static_cast<const FFlecsTestStruct_Value*>(A);
				const FFlecsTestStruct_Value* StructB = static_cast<const FFlecsTestStruct_Value*>(B);
				return StructB->Value - StructA->Value;
			})
			.Build();
		
		{
			static const FFlecsTestStruct_Value V3{ 3 };
			static const FFlecsTestStruct_Value V1{ 1 };
			static const FFlecsTestStruct_Value V2{ 2 };

			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V1);
			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V2);
			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V3);
		}
		
		TArray<int32> ExpectedOrder = { 3, 2, 1 };
		int32 Index = 0;
		Query.each([&](flecs::iter& Iter, size_t IndexInIter)
		{
			const FFlecsTestStruct_Value& Value = Iter.field_at<const FFlecsTestStruct_Value>(0, IndexInIter);
			ASSERT_THAT(IsTrue(Value.Value == ExpectedOrder[Index]));
			Index++;
		});
	}
	
	TEST_METHOD(C9_BuilderAPI_OrderBy_ScriptStructValue_Ascending_CPPAPI)
	{
		FFlecsQuery Query = FlecsWorld->CreateQueryBuilder()
			.With<FFlecsTestStruct_Value>()
			.OrderBy<FFlecsTestStruct_Value>(
				[](const FFlecsId IdA, const FFlecsTestStruct_Value* A, const FFlecsId IdB, const FFlecsTestStruct_Value* B) -> int32
			{
				return A->Value - B->Value;
			})
			.Build();
		
		{
			static const FFlecsTestStruct_Value V3{ 3 };
			static const FFlecsTestStruct_Value V1{ 1 };
			static const FFlecsTestStruct_Value V2{ 2 };

			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V1);
			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V2);
			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V3);
		}

		TArray<int32> ExpectedOrder = { 1, 2, 3 };
		int32 Index = 0;
		Query.each([&](flecs::iter& Iter, size_t IndexInIter)
		{
			const FFlecsTestStruct_Value& Value = Iter.field_at<const FFlecsTestStruct_Value>(0, IndexInIter);
			ASSERT_THAT(IsTrue(Value.Value == ExpectedOrder[Index]));
			Index++;
		});
	}
	
	TEST_METHOD(C10_BuilderAPI_OrderBy_ScriptStructValue_Descending_CPPAPI)
	{
		FFlecsQuery Query = FlecsWorld->CreateQueryBuilder()
			.With<FFlecsTestStruct_Value>()
			.OrderBy<FFlecsTestStruct_Value>(
				[](const FFlecsId IdA, const FFlecsTestStruct_Value* A, const FFlecsId IdB, const FFlecsTestStruct_Value* B) -> int32
			{
				return B->Value - A->Value;
			})
			.Build();
		
		{
			static const FFlecsTestStruct_Value V3{ 3 };
			static const FFlecsTestStruct_Value V1{ 1 };
			static const FFlecsTestStruct_Value V2{ 2 };

			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V1);
			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V2);
			FlecsWorld->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V3);
		}
		
		TArray<int32> ExpectedOrder = { 3, 2, 1 };
		int32 Index = 0;
		Query.each([&](flecs::iter& Iter, size_t IndexInIter)
		{
			const FFlecsTestStruct_Value& Value = Iter.field_at<const FFlecsTestStruct_Value>(0, IndexInIter);
			ASSERT_THAT(IsTrue(Value.Value == ExpectedOrder[Index]));
			Index++;
		});
	}
	
	/*TEST_METHOD(D1_GroupBy_DontFragmentParentHierarchy_Cascading)
	{
		FFlecsEntityHandle Root = FlecsWorld->CreateEntity()
		.Set<FFlecsTestStruct_Value>({ 1 });
		ASSERT_THAT(IsTrue(Root.IsValid()));
		ASSERT_THAT(IsTrue(Root.Has<FFlecsTestStruct_Value>()));

		FFlecsEntityHandle Parent = FlecsWorld->CreateEntity()
			.Set<FFlecsTestStruct_Value>({ 10 });
		ASSERT_THAT(IsTrue(Parent.IsValid()));
		ASSERT_THAT(IsTrue(Parent.Has<FFlecsTestStruct_Value>()));

		FFlecsEntityHandle Child = FlecsWorld->CreateEntity()
			.Set<FFlecsTestStruct_Value>({ 100 });
		ASSERT_THAT(IsTrue(Root.IsValid()));
		ASSERT_THAT(IsTrue(Child.Has<FFlecsTestStruct_Value>()));
		
		Parent.SetParent(Root);
		Child.SetParent(Parent);
		
		FFlecsQuery Query = FlecsWorld->CreateQueryBuilder()
			.With<FFlecsTestStruct_Value>()
			.With<FFlecsTestStruct_Value>().Up()
			.GroupBy(flecs::ParentDepth)
			.Build();
		
		ASSERT_THAT(IsTrue(Query.IsValid()));
		
		TArray<int32> ExpectedOrder = { 1, 10, 100 };
		int32 Index = 0;
		Query.each([&](flecs::iter& Iter, size_t IndexInIter)
		{
			const FFlecsTestStruct_Value& Value = Iter.field_at<const FFlecsTestStruct_Value>(0, IndexInIter);
			ASSERT_THAT(IsTrue(Value.Value == ExpectedOrder[Index]));
			Index++;
		});
		
		for (int Depth = 0; Depth < 8; ++Depth)
		{
			Query.set_group(Depth).each([](FFlecsTestStruct_Value& V)
			{
				const FFlecsTestStruct_Value* ParentV = P.get<FFlecsTestStruct_Value>();
				// In a correct hierarchy, this should always exist
				check(ParentV);
				V.Value += ParentV->Value;
			});
		}

		const FFlecsTestStruct_Value& ParentOut = Parent.Get<FFlecsTestStruct_Value>();
		const FFlecsTestStruct_Value& ChildOut  = Child.Get<FFlecsTestStruct_Value>();

		ASSERT_THAT(AreEqual(ParentOut.Value, 11));
		ASSERT_THAT(AreEqual(ChildOut.Value, 111));
	}*/

}; // End of B2_UnrealFlecsQueryDefinitionTests

#endif // WITH_AUTOMATION_TESTS
