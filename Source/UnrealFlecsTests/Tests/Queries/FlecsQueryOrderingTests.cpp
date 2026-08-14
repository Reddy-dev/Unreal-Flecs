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

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS(FlecsQueryOrderingTests,
								   "UnrealFlecs.Queries.Ordering",
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
	TEST_METHOD(BuilderAPI_OrderByCallbackDefinition_ScriptStructValue_Ascending_ScriptStructAPI)
	{
		FFlecsQuery Query = World()->CreateQueryBuilder()
			.With(FFlecsTestStruct_Value::StaticStruct())
			.OrderByCallbackDefinition(FFlecsTestStruct_Value::StaticStruct(), 
				TInstancedStruct<FFlecsQueryOrderByCallbackDefinitionTest_ScriptStruct>::Make(EFlecsTestQueryOrderByFunction::Ascending))
			.Build();
		
		{
			static const FFlecsTestStruct_Value V3{ 3 };
			static const FFlecsTestStruct_Value V1{ 1 };
			static const FFlecsTestStruct_Value V2{ 2 };

			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V1);
			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V2);
			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V3);
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
	
	TEST_METHOD(BuilderAPI_OrderByCallbackDefinition_ScriptStructValue_Descending_ScriptStructAPI)
	{
		FFlecsQuery Query = World()->CreateQueryBuilder()
			.With(FFlecsTestStruct_Value::StaticStruct())
			.OrderByCallbackDefinition(FFlecsTestStruct_Value::StaticStruct(), 
				TInstancedStruct<FFlecsQueryOrderByCallbackDefinitionTest_ScriptStruct>::Make(EFlecsTestQueryOrderByFunction::Descending))
			.Build();
		
		{
			static const FFlecsTestStruct_Value V3{ 3 };
			static const FFlecsTestStruct_Value V1{ 1 };
			static const FFlecsTestStruct_Value V2{ 2 };

			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V1);
			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V2);
			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V3);
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
	
	TEST_METHOD(BuilderAPI_OrderByCallbackDefinition_ScriptStructValue_Ascending_CPPAPI)
	{
		FFlecsQuery Query = World()->CreateQueryBuilder()
			.With(FFlecsTestStruct_Value::StaticStruct())
			.OrderByCallbackDefinition<FFlecsTestStruct_Value>(
				TInstancedStruct<FFlecsQueryOrderByCallbackDefinitionTest_ScriptStruct>::Make(EFlecsTestQueryOrderByFunction::Ascending))
			.Build();
		
		{
			static const FFlecsTestStruct_Value V3{ 3 };
			static const FFlecsTestStruct_Value V1{ 1 };
			static const FFlecsTestStruct_Value V2{ 2 };

			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V1);
			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V2);
			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V3);
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
	
	TEST_METHOD(BuilderAPI_OrderByCallbackDefinition_ScriptStructValue_Descending_CPPAPI)
	{
		FFlecsQuery Query = World()->CreateQueryBuilder()
			.With(FFlecsTestStruct_Value::StaticStruct())
			.OrderByCallbackDefinition<FFlecsTestStruct_Value>(
				TInstancedStruct<FFlecsQueryOrderByCallbackDefinitionTest_ScriptStruct>::Make(EFlecsTestQueryOrderByFunction::Descending))
			.Build();
		
		{
			static const FFlecsTestStruct_Value V3{ 3 };
			static const FFlecsTestStruct_Value V1{ 1 };
			static const FFlecsTestStruct_Value V2{ 2 };

			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V1);
			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V2);
			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V3);
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
	
	TEST_METHOD(BuilderAPI_OrderByOrderByCallbackDefinition_CppOnlyStructValue_Ascending_CPPAPI)
	{
		FFlecsQuery Query = World()->CreateQueryBuilder()
			.With<FFlecsTest_CPPStructValue>()
			.OrderByCallbackDefinition<FFlecsTest_CPPStructValue>(
				TInstancedStruct<FFlecsQueryOrderByCallbackDefinitionTest_CPPType>::Make(EFlecsTestQueryOrderByFunction::Ascending))
			.Build();
		
		{
			static const FFlecsTest_CPPStructValue V3{ 3 };
			static const FFlecsTest_CPPStructValue V1{ 1 };
			static const FFlecsTest_CPPStructValue V2{ 2 };

			World()->CreateEntity().Set<FFlecsTest_CPPStructValue>(V1);
			World()->CreateEntity().Set<FFlecsTest_CPPStructValue>(V2);
			World()->CreateEntity().Set<FFlecsTest_CPPStructValue>(V3);
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
	
	TEST_METHOD(BuilderAPI_OrderByOrderByCallbackDefinition_CppOnlyStructValue_Descending_CPPAPI)
	{
		FFlecsQuery Query = World()->CreateQueryBuilder()
			.With<FFlecsTest_CPPStructValue>()
			.OrderByCallbackDefinition<FFlecsTest_CPPStructValue>(
				TInstancedStruct<FFlecsQueryOrderByCallbackDefinitionTest_CPPType>::Make(EFlecsTestQueryOrderByFunction::Descending))
			.Build();
		
		{
			static const FFlecsTest_CPPStructValue V3{ 3 };
			static const FFlecsTest_CPPStructValue V1{ 1 };
			static const FFlecsTest_CPPStructValue V2{ 2 };

			World()->CreateEntity().Set<FFlecsTest_CPPStructValue>(V1);
			World()->CreateEntity().Set<FFlecsTest_CPPStructValue>(V2);
			World()->CreateEntity().Set<FFlecsTest_CPPStructValue>(V3);
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
	
	TEST_METHOD(BuilderAPI_OrderBy_ScriptStructValue_Ascending_StringAPI)
	{
		FFlecsQuery Query = World()->CreateQueryBuilder()
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

			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V1);
			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V2);
			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V3);
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
	
	TEST_METHOD(BuilderAPI_OrderBy_ScriptStructValue_Descending_StringAPI)
	{
		FFlecsQuery Query = World()->CreateQueryBuilder()
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

			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V1);
			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V2);
			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V3);
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
	
	TEST_METHOD(BuilderAPI_OrderBy_ScriptStructValue_Ascending_CPPAPI)
	{
		FFlecsQuery Query = World()->CreateQueryBuilder()
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

			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V1);
			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V2);
			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V3);
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
	
	TEST_METHOD(BuilderAPI_OrderBy_ScriptStructValue_Descending_CPPAPI)
	{
		FFlecsQuery Query = World()->CreateQueryBuilder()
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

			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V1);
			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V2);
			World()->CreateEntity().Set(FFlecsTestStruct_Value::StaticStruct(), &V3);
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
	
	
}; // FlecsQueryOrderingTests

#endif // WITH_AUTOMATION_TESTS
