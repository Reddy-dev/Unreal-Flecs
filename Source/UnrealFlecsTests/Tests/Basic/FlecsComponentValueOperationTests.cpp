// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Worlds/FlecsWorld.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsComponentValueOperationTests,
								 "UnrealFlecs.Components.Operations.Values",
                               EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter,
                               "[Flecs][Entity][Tag][Component]")
{
protected:
	virtual void OnRegisteredWorldSetUp() override
	{
		TagEntity = World()->RegisterComponentType<FFlecsTestStruct_Tag>();
		ValuedComponentEntity = World()->RegisterComponentType<FFlecsTestStruct_Value>();

		ToggleableComponentHandle = World()->RegisterComponentType<FFlecsTestStruct_Toggleable>();

		TestEntity = World()->CreateEntity("TestEntity");
		ToggleableEntityTest = World()->CreateEntity("ToggleableEntityTest")
			.Add<FFlecsTestStruct_Toggleable>();

		Query = World()->GetNativeFlecsWorld().query_builder<FFlecsTestStruct_Value>()
			.cached()
			.detect_changes()
			.build();
	}

	virtual void OnWorldTearDown() override
	{
		Query.destruct();
		Query = {};
	}

private:
	FFlecsEntityHandle TagEntity;
	FFlecsEntityHandle ValuedComponentEntity;
	FFlecsComponentHandle ToggleableComponentHandle;
	FFlecsEntityHandle TestEntity;
	FFlecsEntityHandle ToggleableEntityTest;
	flecs::query<FFlecsTestStruct_Value> Query;

public:
	TEST_METHOD(BasicComponentSetRemove_Set_CPPAPI_Remove_CPPAPI)
	{
		TestEntity.Set<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ .Value = 42 });
		ASSERT_THAT(IsTrue(TestEntity.Has(ValuedComponentEntity)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));

		const auto& [Value] = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Value == 42));

		const void* ValuePtr = TestEntity.TryGet(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsTrue(ValuePtr != nullptr));
		const TSolidNotNull<const FFlecsTestStruct_Value*> ValuePtrTyped = static_cast<const FFlecsTestStruct_Value*>(ValuePtr);
		ASSERT_THAT(IsTrue(ValuePtrTyped->Value == 42));

		TestEntity.Remove<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsFalse(TestEntity.Has(ValuedComponentEntity)));
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsFalse(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));
	}

	TEST_METHOD(BasicComponentSetRemove_Set_StaticStructAPI_Remove_StaticStructAPI)
	{
		constexpr FFlecsTestStruct_Value TestValue{ .Value = 42 };
		
		TestEntity.Set(FFlecsTestStruct_Value::StaticStruct(), &TestValue);
		ASSERT_THAT(IsTrue(TestEntity.Has(ValuedComponentEntity)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));

		const auto& [Value] = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Value == 42));

		const void* ValuePtr = TestEntity.TryGet(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsTrue(ValuePtr != nullptr));
		
		const TSolidNotNull<const FFlecsTestStruct_Value*> ValuePtrTyped = static_cast<const FFlecsTestStruct_Value*>(ValuePtr);
		ASSERT_THAT(IsTrue(ValuePtrTyped->Value == 42));

		TestEntity.Remove(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsFalse(TestEntity.Has(ValuedComponentEntity)));
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsFalse(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));
	}

	TEST_METHOD(BasicComponentSetRemove_Set_StaticStructAPI_Remove_CPPAPI)
	{
		constexpr FFlecsTestStruct_Value TestValue{ .Value = 42 };
		
		TestEntity.Set(FFlecsTestStruct_Value::StaticStruct(), &TestValue);
		ASSERT_THAT(IsTrue(TestEntity.Has(ValuedComponentEntity)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));

		const auto& [Value] = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Value == 42));

		const void* ValuePtr = TestEntity.TryGet(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsTrue(ValuePtr != nullptr));
		
		const TSolidNotNull<const FFlecsTestStruct_Value*> ValuePtrTyped = static_cast<const FFlecsTestStruct_Value*>(ValuePtr);
		ASSERT_THAT(IsTrue(ValuePtrTyped->Value == 42));

		TestEntity.Remove<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsFalse(TestEntity.Has(ValuedComponentEntity)));
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsFalse(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));
	}

	TEST_METHOD(BasicComponentSetRemove_Set_CPPAPI_Remove_StaticStructAPI)
	{
		TestEntity.Set<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ .Value = 42 });
		ASSERT_THAT(IsTrue(TestEntity.Has(ValuedComponentEntity)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));

		const auto& [Value] = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Value == 42));

		const void* ValuePtr = TestEntity.TryGet(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsTrue(ValuePtr != nullptr));
		
		const TSolidNotNull<const FFlecsTestStruct_Value*> ValuePtrTyped = static_cast<const FFlecsTestStruct_Value*>(ValuePtr);
		ASSERT_THAT(IsTrue(ValuePtrTyped->Value == 42));

		TestEntity.Remove(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsFalse(TestEntity.Has(ValuedComponentEntity)));
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsFalse(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));
	}

	TEST_METHOD(BasicComponentSetRemove_Set_EntityAPI_Remove_StaticStructAPI)
	{
		static constexpr FFlecsTestStruct_Value TestValue{ .Value = 42 };
		
		TestEntity.Set(ValuedComponentEntity, &TestValue);
		ASSERT_THAT(IsTrue(TestEntity.Has(ValuedComponentEntity)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));

		const auto& [Value] = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Value == 42));

		const void* ValuePtr = TestEntity.TryGet(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsTrue(ValuePtr != nullptr));
		
		const TSolidNotNull<const FFlecsTestStruct_Value*> ValuePtrTyped = static_cast<const FFlecsTestStruct_Value*>(ValuePtr);
		ASSERT_THAT(IsTrue(ValuePtrTyped->Value == 42));

		TestEntity.Remove(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsFalse(TestEntity.Has(ValuedComponentEntity)));
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsFalse(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));
	}

	TEST_METHOD(BasicComponentAddAssignRemove_Add_CPPAPI_Assign_CPPAPI_Remove_CPPAPI)
	{
		static constexpr int32 StartingValue = 1;
		
		TestEntity.Add<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(TestEntity.Has(ValuedComponentEntity)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));

		const auto& [OldValue] = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(OldValue == StartingValue));
		
		TestEntity.Assign<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ .Value = 42 });

		const auto& [Value] = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Value == 42));

		const void* ValuePtr = TestEntity.TryGet(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsTrue(ValuePtr != nullptr));
		
		const TSolidNotNull<const FFlecsTestStruct_Value*> ValuePtrTyped = static_cast<const FFlecsTestStruct_Value*>(ValuePtr);
		ASSERT_THAT(IsTrue(ValuePtrTyped->Value == 42));

		TestEntity.Remove<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsFalse(TestEntity.Has(ValuedComponentEntity)));
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsFalse(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));
	}

	TEST_METHOD(BasicComponentAddAssignRemove_Add_StaticStructAPI_Assign_StaticStructAPI_Remove_StaticStructAPI)
	{
		static constexpr int32 StartingValue = 1;

		static constexpr FFlecsTestStruct_Value DefaultValue{ .Value = 42 };
		
		TestEntity.Add(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsTrue(TestEntity.Has(ValuedComponentEntity)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));

		const auto& [OldValue] = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(OldValue == StartingValue));
		
		TestEntity.Assign(FFlecsTestStruct_Value::StaticStruct(), &DefaultValue);

		const auto& [Value] = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Value == 42));

		const void* ValuePtr = TestEntity.TryGet(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsTrue(ValuePtr != nullptr));
		
		const TSolidNotNull<const FFlecsTestStruct_Value*> ValuePtrTyped = static_cast<const FFlecsTestStruct_Value*>(ValuePtr);
		ASSERT_THAT(IsTrue(ValuePtrTyped->Value == 42));

		TestEntity.Remove(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsFalse(TestEntity.Has(ValuedComponentEntity)));
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsFalse(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));
	}

	TEST_METHOD(BasicComponentAddAssignRemove_Add_EntityAPI_Assign_EntityAPI_Remove_EntityAPI)
	{
		static constexpr int32 StartingValue = 1;

		static constexpr FFlecsTestStruct_Value DefaultValue{ .Value = 42 };
		
		TestEntity.Add(ValuedComponentEntity);
		ASSERT_THAT(IsTrue(TestEntity.Has(ValuedComponentEntity)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));

		const auto& [OldValue] = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(OldValue == StartingValue));
		
		TestEntity.Assign(ValuedComponentEntity, &DefaultValue);

		const auto& [Value] = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Value == 42));

		const void* ValuePtr = TestEntity.TryGet(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsTrue(ValuePtr != nullptr));
		
		const TSolidNotNull<const FFlecsTestStruct_Value*> ValuePtrTyped = static_cast<const FFlecsTestStruct_Value*>(ValuePtr);
		ASSERT_THAT(IsTrue(ValuePtrTyped->Value == 42));

		TestEntity.Remove(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsFalse(TestEntity.Has(ValuedComponentEntity)));
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsFalse(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));
	}

}; // FlecsComponentValueOperationTests


#endif // #if WITH_AUTOMATION_TESTS
