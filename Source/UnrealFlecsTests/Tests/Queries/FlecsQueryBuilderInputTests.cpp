// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "FlecsQueryDefinitionTestTypes.h"
#include "Queries/FlecsQuery.h"
#include "Queries/FlecsQueryBuilder.h"
#include "Queries/FlecsQueryBuilderView.h"

#include "Queries/FlecsQueryDefinition.h"
#include "Queries/Generator/FlecsQueryGeneratorInputType.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS(FlecsQueryBuilderInputTests,
								   "UnrealFlecs.Queries.BuilderInputs",
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
	TEST_METHOD(BuilderConstruction_ReadWriteVariants_AddStagedTerms_CPPAPI)
	{
		FFlecsQueryBuilder Builder = World()->CreateQueryBuilder();
		Builder
			.Read<FFlecsTestStruct_Value>()
			.Write<FFlecsTest_CPPStructValue>()
			.ReadWrite<FFlecsTestStruct_PairIsTag>();

		const FFlecsQueryDefinition& Definition = Builder.GetQueryDefinition();
		ASSERT_THAT(IsTrue(Definition.Terms.Num() == 3));

		ASSERT_THAT(IsTrue(Definition.Terms[0].InOut == EFlecsQueryInOut::Read));
		ASSERT_THAT(IsTrue(Definition.Terms[1].InOut == EFlecsQueryInOut::Write));
		ASSERT_THAT(IsTrue(Definition.Terms[2].InOut == EFlecsQueryInOut::ReadWrite));

		ASSERT_THAT(IsTrue(Definition.Terms[0].bStage));
		ASSERT_THAT(IsTrue(Definition.Terms[1].bStage));
		ASSERT_THAT(IsTrue(Definition.Terms[2].bStage));
	}

	TEST_METHOD(BuilderConstruction_ReadWriteVariants_AddStagedTerms_InputAPI)
	{
		FFlecsQueryBuilder Builder = World()->CreateQueryBuilder();
		Builder
			.Read(FFlecsTestStruct_Value::StaticStruct())
			.Write(FString(TEXT("FFlecsTest_CPPStructValue")))
			.ReadWrite(StaticEnum<EFlecsTestEnum_UENUM>());

		const FFlecsQueryDefinition& Definition = Builder.GetQueryDefinition();
		ASSERT_THAT(IsTrue(Definition.Terms.Num() == 3));

		ASSERT_THAT(IsTrue(Definition.Terms[0].InOut == EFlecsQueryInOut::Read));
		ASSERT_THAT(IsTrue(Definition.Terms[1].InOut == EFlecsQueryInOut::Write));
		ASSERT_THAT(IsTrue(Definition.Terms[2].InOut == EFlecsQueryInOut::ReadWrite));

		ASSERT_THAT(IsTrue(Definition.Terms[0].bStage));
		ASSERT_THAT(IsTrue(Definition.Terms[1].bStage));
		ASSERT_THAT(IsTrue(Definition.Terms[2].bStage));
	}

	TEST_METHOD(BuilderConstruction_WithScriptStructTagTerm_ScriptStructAPI)
	{
		FFlecsQuery Query = World()->CreateQueryBuilder()
			.With(FFlecsTestStruct_Tag::StaticStruct())
			.Build();
		
		ASSERT_THAT(IsTrue(Query.IsValid()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = World()->CreateEntity()
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
	
	TEST_METHOD(BuilderConstruction_WithScriptStructTagTerm_CPPAPI)
	{
		FFlecsQuery Query = World()->CreateQueryBuilder()
			.With<FFlecsTestStruct_Tag>()
			.Build();
		
		ASSERT_THAT(IsTrue(Query.IsValid()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = World()->CreateEntity()
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
	
	TEST_METHOD(BuilderConstruction_WithScriptStructTagTerm_StringAPI)
	{
		FFlecsQuery Query = World()->CreateQueryBuilder()
			.With("FFlecsTestStruct_Tag")
			.Build();
		
		ASSERT_THAT(IsTrue(Query.IsValid()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = World()->CreateEntity()
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
	
	TEST_METHOD(BuilderConstruction_WithScriptStructTagTermAndWithoutTag_ScriptStructAPI)
	{
		FFlecsQuery Query = World()->CreateQueryBuilder()
			.Without(FFlecsTestStruct_Tag::StaticStruct())
			.With(FFlecsTestStruct_Value::StaticStruct())
			.Build();
		
		ASSERT_THAT(IsTrue(Query.IsValid()));
		
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
	
	TEST_METHOD(BuilderConstruction_WithCPPTagTermAndWithoutTag_CPPAPI)
	{
		FFlecsQuery Query = World()->CreateQueryBuilder()
			.Without<FFlecsTestStruct_Tag>()
			.With<FFlecsTest_CPPStructValue>()
			.Build();
		
		ASSERT_THAT(IsTrue(Query.IsValid()));
		
		static const FFlecsTest_CPPStructValue TestValue { 256 };
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Set<FFlecsTest_CPPStructValue>(TestValue);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTest_CPPStructValue>()));
		
		const FFlecsTest_CPPStructValue& RetrievedValue = TestEntity.Get<FFlecsTest_CPPStructValue>();
		ASSERT_THAT(IsTrue(RetrievedValue.Value == 256));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = World()->CreateEntity()
			.Add<FFlecsTestStruct_Tag>()
			.Set<FFlecsTest_CPPStructValue>(TestValue);
		ASSERT_THAT(IsTrue(TestEntity2.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity2.Has<FFlecsTest_CPPStructValue>()));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
	}
	
	TEST_METHOD(BuilderConstruction_WithTypedQueryDefinition_ScriptStructTagTerm_ScriptStructAPI)
	{
		TTypedFlecsQuery<FFlecsTestStruct_Tag> Query = World()->CreateQueryBuilder<FFlecsTestStruct_Tag>()
			.Build();
		
		ASSERT_THAT(IsTrue(Query.IsValid()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = World()->CreateEntity()
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
	
	TEST_METHOD(BuilderConstruction_WithTypedQueryDefinition_ScriptStructTagTermAndScriptStructValueTerm_ScriptStructAPI)
	{
		TTypedFlecsQuery<FFlecsTestStruct_Value> Query = World()->CreateQueryBuilder<const FFlecsTestStruct_Value>()
			.With<FFlecsTestStruct_Tag>()
			.Build();
		
		ASSERT_THAT(IsTrue(Query.IsValid()));
		
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Add<FFlecsTestStruct_Tag>()
			.Set<FFlecsTestStruct_Value>({ 512 });
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		
		ASSERT_THAT(IsTrue(Query.is_true()));
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		FFlecsEntityHandle TestEntity2 = World()->CreateEntity()
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
	
	TEST_METHOD(BuilderConstruction_WithTypedQueryDefinition_CPPOnlyValueTerm_WithInOutRef_CPPAPI)
	{
		TTypedFlecsQuery<FFlecsTest_CPPStructValue> Query = World()->CreateQueryBuilder<FFlecsTest_CPPStructValue&>()
			.Build();
		
		ASSERT_THAT(IsTrue(Query.IsValid()));
		
		static const FFlecsTest_CPPStructValue TestValue { 256 };
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Set(TestValue);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTest_CPPStructValue>()));
		
		const FFlecsTest_CPPStructValue& RetrievedValue = TestEntity.Get<FFlecsTest_CPPStructValue>();
		ASSERT_THAT(IsTrue(RetrievedValue.Value == 256));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		int32 FoundValueSum = 0;
		Query.each([&](flecs::iter& Iter, size_t Index, FFlecsTest_CPPStructValue& Value)
		{
			Value.Value += 1;
			FoundValueSum += Value.Value;
		});
		
		ASSERT_THAT(IsTrue(FoundValueSum == 257));
		ASSERT_THAT(IsTrue(TestEntity.Get<FFlecsTest_CPPStructValue>().Value == 257));
	}
	
	TEST_METHOD(BuilderConstruction_WithTypedQueryDefinition_CPPOnlyValueTerm_WithInConstRef_CPPAPI)
	{
		TTypedFlecsQuery<FFlecsTest_CPPStructValue> Query = World()->CreateQueryBuilder<const FFlecsTest_CPPStructValue>()
			.Build();
		
		ASSERT_THAT(IsTrue(Query.IsValid()));
		
		static const FFlecsTest_CPPStructValue TestValue { 256 };
		FFlecsEntityHandle TestEntity = World()->CreateEntity()
			.Set(TestValue);
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTest_CPPStructValue>()));
		
		const FFlecsTest_CPPStructValue& RetrievedValue = TestEntity.Get<FFlecsTest_CPPStructValue>();
		ASSERT_THAT(IsTrue(RetrievedValue.Value == 256));
		
		ASSERT_THAT(IsTrue(Query.count() == 1));
		
		int32 FoundValueSum = 0;
		Query.each([&](flecs::iter& Iter, size_t Index, const FFlecsTest_CPPStructValue& Value)
		{
			FoundValueSum += Value.Value;
		});
		
		ASSERT_THAT(IsTrue(FoundValueSum == 256));
		ASSERT_THAT(IsTrue(TestEntity.Get<FFlecsTest_CPPStructValue>().Value == 256));
	}
	
}; // FlecsQueryBuilderInputTests

#endif // WITH_AUTOMATION_TESTS
