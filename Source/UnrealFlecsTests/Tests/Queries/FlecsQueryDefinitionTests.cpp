// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#include "Queries/FlecsQueryDefinition.h"

// @TODO: add pair testing

/**
 * Layout of the tests:
 * A. Construction Tests
 **/
TEST_CLASS_WITH_FLAGS(B2_UnrealFlecsQueryDefinitionTests,
							   "UnrealFlecs.B2_Query_Definition",
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
	
	TEST_METHOD(A2_Construction_WithScriptStructTagTerm_CPPAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		TermExpression1.SetInput<FFlecsTestStruct_Tag>();
		
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
	
	TEST_METHOD(A3_Construction_WithScriptStructValueTerm_CPPAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		TermExpression1.SetInput<FFlecsTestStruct_Value>();
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Set<FFlecsTestStruct_Value>({ 61 });
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Get<FFlecsTestStruct_Value>().Value == 61));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		Query.each([&](flecs::iter& Iter, size_t Index)
		{
			const FFlecsTestStruct_Value& Value = Iter.field_at<FFlecsTestStruct_Value>(0, Index);
			ASSERT_THAT(AreEqual(Value.Value, 61));
		});
	}
	
	TEST_METHOD(A4_Construction_WithScriptStructTagTerm_ScriptStructAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		TermExpression1.SetInput(FFlecsTestStruct_Tag::StaticStruct());
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add(FFlecsTestStruct_Tag::StaticStruct());
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Tag::StaticStruct())));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(A5_Construction_WithScriptStructValueTerm_ScriptStructAPI)
	{
		static const FFlecsTestStruct_Value TestValue { 84 };
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		TermExpression1.SetInput(FFlecsTestStruct_Value::StaticStruct());
		
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
	
	TEST_METHOD(A6_Construction_WithCPPStructTagTerm_CPPAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		TermExpression1.SetInput<FFlecsTest_CPPStruct>();
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add<FFlecsTest_CPPStruct>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTest_CPPStruct>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(A7_Construction_WithCPPStructValueTerm_CPPAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		TermExpression1.SetInput<FFlecsTest_CPPStructValue>();
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Set<FFlecsTest_CPPStructValue>({42});
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTest_CPPStructValue>()));
		ASSERT_THAT(IsTrue(TestEntity.Get<FFlecsTest_CPPStructValue>().Value == 42));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		Query.each([&](flecs::iter& Iter, size_t Index)
		{
			const FFlecsTest_CPPStructValue& Value = Iter.field_at<FFlecsTest_CPPStructValue>(0, Index);
			ASSERT_THAT(AreEqual(Value.Value, 42));
		});
	}
	
	TEST_METHOD(A8_Construction_WithGameplayTagTerm)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		TermExpression1.SetInput(FFlecsTestNativeGameplayTags::StaticInstance.TestTag1);
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add(FFlecsTestNativeGameplayTags::StaticInstance.TestTag1);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestNativeGameplayTags::StaticInstance.TestTag1)));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(A9_Construction_WithStringTerm)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		TermExpression1.SetInput(TEXT("FFlecsTestStruct_Tag"));
		
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
	
	TEST_METHOD(A10_Construction_WithCPPAPITerm)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		TermExpression1.SetInput<FFlecsTestStruct_Tag>();
		
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
	
	TEST_METHOD(A11_Construction_WithVariableTerm)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		TermExpression1.SetInput("$MyVarTag");
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		
		FFlecsComponentHandle TagComponentHandle = FlecsWorld->ObtainComponentTypeStruct<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TagComponentHandle.IsValid()));
		
		Query.set_var("MyVarTag", TagComponentHandle).each([&](flecs::iter& Iter, size_t Index)
		{
			FFlecsEntityHandle Entity = Iter.entity(Index);
			ASSERT_THAT(AreEqual(Entity, TestEntity));
		});
	}
	
	TEST_METHOD(A12_Construction_WithPairTerm_CPPAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		TermExpression1.SetInput<FFlecsTestStruct_PairIsTag>();
		TermExpression1.SetSecondInput<FFlecsTestStruct_Tag>();
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.AddPair<FFlecsTestStruct_PairIsTag, FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FFlecsTestStruct_PairIsTag, FFlecsTestStruct_Tag>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(A13_Construction_WithPairTerm_ScriptStructAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		TermExpression1.SetInput(FFlecsTestStruct_PairIsTag::StaticStruct());
		TermExpression1.SetSecondInput(FFlecsTestStruct_Tag::StaticStruct());
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.AddPair(FFlecsTestStruct_PairIsTag::StaticStruct(), FFlecsTestStruct_Tag::StaticStruct());
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasPair(FFlecsTestStruct_PairIsTag::StaticStruct(), FFlecsTestStruct_Tag::StaticStruct())));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(A14_Construction_WithPairTermAndWildcard_CPPAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		TermExpression1.SetInput<FFlecsTestStruct_PairIsTag>();
		TermExpression1.SetSecondInput(flecs::Wildcard);
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity1 = FlecsWorld->CreateEntity()
			.AddPair<FFlecsTestStruct_PairIsTag, FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity1.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity1.HasPair<FFlecsTestStruct_PairIsTag, FFlecsTestStruct_Tag>()));
		
		FFlecsEntityHandle TestEntity2 = FlecsWorld->CreateEntity()
			.AddPair<FFlecsTestStruct_PairIsTag, FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.HasPair<FFlecsTestStruct_PairIsTag, FFlecsTestStruct_Value>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 2));
	}
	
	TEST_METHOD(A15_Construction_WithPairTermAndVariable_ScriptStructAPI)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		TermExpression1.SetInput(FFlecsTestStruct_PairIsTag::StaticStruct());
		TermExpression1.SetSecondInput(TEXT("$MyVarTag"));
		
		QueryDefinition.Terms.Add(TermExpression1);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.AddPair(FFlecsTestStruct_PairIsTag::StaticStruct(), FFlecsTestStruct_Tag::StaticStruct());
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasPair(FFlecsTestStruct_PairIsTag::StaticStruct(), FFlecsTestStruct_Tag::StaticStruct())));
		
		FFlecsEntityHandle TestEntity2 = FlecsWorld->CreateEntity()
			.AddPair<FFlecsTestStruct_PairIsTag, FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.HasPair<FFlecsTestStruct_PairIsTag, FFlecsTestStruct_Value>()));
		
		FFlecsComponentHandle TagComponentHandle = FlecsWorld->ObtainComponentTypeStruct<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TagComponentHandle.IsValid()));
		
		ASSERT_THAT(IsTrue(Query.count() == 2));
		
		Query.set_var("MyVarTag", TagComponentHandle).each([&](flecs::iter& Iter, size_t Index)
		{
			FFlecsEntityHandle Entity = Iter.entity(Index);
			ASSERT_THAT(AreEqual(Entity, TestEntity));
		});
	}
	
	TEST_METHOD(A16_Construction_WithMultipleTerms)
	{
		FFlecsQueryDefinition QueryDefinition;
		
		FFlecsQueryTermExpression TermExpression1;
		TermExpression1.SetInput<FFlecsTestStruct_Tag>();
		
		FFlecsQueryTermExpression TermExpression2;
		TermExpression2.SetInput<FFlecsTestStruct_Value>();
		
		QueryDefinition.Terms.Add(TermExpression1);
		QueryDefinition.Terms.Add(TermExpression2);
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
		
		FFlecsEntityHandle TestEntity = FlecsWorld->CreateEntity()
			.Add<FFlecsTestStruct_Tag>()
			.Set<FFlecsTestStruct_Value>({ 123 });
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}

}; // End of B2_UnrealFlecsQueryDefinitionTests

#endif // WITH_AUTOMATION_TESTS
