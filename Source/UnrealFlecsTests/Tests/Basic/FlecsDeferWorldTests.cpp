// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "StructUtils/StructView.h"
#include "Worlds/FlecsWorld.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS_AND_TAGS(DeferWorldTests, "UnrealFlecs.World.Deferred",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
	| EAutomationTestFlags::CriticalPriority, "[Flecs]")
{
protected:
	virtual void OnRegisteredWorldSetUp() override
	{
		TestEntity = World()->CreateEntity("TestEntity");
		solid_checkf(TestEntity.IsValid(), TEXT("Failed to create TestEntity!"));
		
		TestComponent = World()->RegisterComponentType<FFlecsTestStruct_Value>();
		solid_checkf(TestComponent.IsValid(), TEXT("Failed to register TestComponent!"));
	}

private:
	FFlecsEntityHandle TestEntity;
	FFlecsComponentHandle TestComponent;

public:
	TEST_METHOD(AddRemoveComponentDeferred_Add_CPPAPI_Remove_CPPAPI)
	{
		World()->Defer([&]()
		{
			TestEntity.Add<FFlecsTestStruct_Value>();
			
			ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>(),
				"TestEntity should not have the component yet!"));
		});

		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>(),
			"TestEntity should have the component after the deferred context is applied!"));

		World()->Defer([&]()
		{
			TestEntity.Remove<FFlecsTestStruct_Value>();
			
			ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>(),
				"TestEntity should have the component before the deferred context is applied!"));
		});

		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>(),
			"TestEntity should not have the component after the deferred context is applied!"));
	}

	TEST_METHOD(AddRemoveComponentDeferred_Add_StaticStructAPI_Remove_StaticStructAPI)
	{
		World()->Defer([&]()
		{
			TestEntity.Add(FFlecsTestStruct_Value::StaticStruct());
			 
			ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>(),
				"TestEntity should not have the component yet!"));
		});

		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>(),
			"TestEntity should have the component after the deferred context is applied!"));

		World()->Defer([&]()
		{
			TestEntity.Remove(FFlecsTestStruct_Value::StaticStruct());
			
			ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>(),
				"TestEntity should have the component before the deferred context is applied!"));
		});

		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>(),
			"TestEntity should not have the component after the deferred context is applied!"));
	}

	TEST_METHOD(AddRemoveComponentDeferred_Add_EntityAPI_Remove_EntityAPI)
	{
		World()->Defer([&]()
		{
			TestEntity.Add(TestComponent);
			
			ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>(),
				"TestEntity should not have the component yet!"));
		});

		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>(),
			"TestEntity should have the component after the deferred context is applied!"));

		World()->Defer([&]()
		{
			TestEntity.Remove(TestComponent);
			
			ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>(),
				"TestEntity should have the component before the deferred context is applied!"));
		});

		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>(),
			"TestEntity should not have the component after the deferred context is applied!"));
	}

	TEST_METHOD(SetComponentDeferred_Set_CPPAPI)
	{
		World()->Defer([&]()
		{
			TestEntity.Set<FFlecsTestStruct_Value>({ 42 });
			
			ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>(),
				"TestEntity should not have the component yet!"));
		});

		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>(),
			"TestEntity should have the component after the deferred context is applied!"));

		const FFlecsTestStruct_Value& ComponentValue = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(ComponentValue.Value == 42));

		TestEntity.Remove<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>()));
	}

	TEST_METHOD(SetComponentDeferred_Set_StaticStructAPI)
	{
		World()->Defer([&]()
		{
			static constexpr FFlecsTestStruct_Value ComponentValue{ 42 };
			
			TestEntity.Set(FFlecsTestStruct_Value::StaticStruct(), &ComponentValue);
			
			ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>(),
				"TestEntity should not have the component yet!"));
		});

		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>(),
			"TestEntity should have the component after the deferred context is applied!"));

		const FFlecsTestStruct_Value& ComponentValue = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(ComponentValue.Value == 42));

		TestEntity.Remove<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>()));
	}

	TEST_METHOD(ScopedDeferWindow_Add_CPPAPI_Remove_CPPAPI)
	{
		{
			FFlecsScopedDeferWindow DeferWindow(World());
			solid_checkf(DeferWindow.IsValid(), TEXT("DeferWindow is not valid!"));

			TestEntity.Add<FFlecsTestStruct_Value>();
			// 
			ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>(),
				"TestEntity should not have the component yet!"));
		}

		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>(),
			"TestEntity should have the component after the deferred context is applied!"));

		{
			FFlecsScopedDeferWindow DeferWindow(World());
			solid_checkf(DeferWindow.IsValid(), TEXT("DeferWindow is not valid!"));

			TestEntity.Remove<FFlecsTestStruct_Value>();
			
			ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>(),
				"TestEntity should have the component before the deferred context is applied!"));
		}

		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Value>(),
			"TestEntity should not have the component after the deferred context is applied!"));
	}
	
}; // DeferWorldTests

#endif // WITH_AUTOMATION_TESTS
