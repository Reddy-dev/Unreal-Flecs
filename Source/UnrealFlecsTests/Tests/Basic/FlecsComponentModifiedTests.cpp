// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Worlds/FlecsWorld.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsComponentModifiedTests,
								 "UnrealFlecs.Components.Operations.Modified",
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
	TEST_METHOD(BasicComponentModified_Set_CPPAPI)
	{
		TestEntity.Set<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ .Value = 42 });

		ASSERT_THAT(IsTrue(Query.changed()));
		ASSERT_THAT(IsTrue(Query.count() == 1));

		TestEntity.Remove<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Query.changed()));
		ASSERT_THAT(IsTrue(Query.count() == 0));
	}

	TEST_METHOD(BasicComponentModified_Set_StaticStructAPI)
	{
		constexpr FFlecsTestStruct_Value TestValue{ .Value = 42 };
		
		TestEntity.Set(FFlecsTestStruct_Value::StaticStruct(), &TestValue);

		ASSERT_THAT(IsTrue(Query.changed()));
		ASSERT_THAT(IsTrue(Query.count() == 1));

		TestEntity.Remove(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsTrue(Query.changed()));
		ASSERT_THAT(IsTrue(Query.count() == 0));
	}

	TEST_METHOD(BasicComponentModified_Set_EntityAPI)
	{
		static constexpr FFlecsTestStruct_Value TestValue{ .Value = 42 };
		
		TestEntity.Set(ValuedComponentEntity, &TestValue);

		ASSERT_THAT(IsTrue(Query.changed()));
		ASSERT_THAT(IsTrue(Query.count() == 1));

		TestEntity.Remove(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsTrue(Query.changed()));
		ASSERT_THAT(IsTrue(Query.count() == 0));
	}

	TEST_METHOD(BasicComponentModified_Modified_CPPAPI)
	{
		TestEntity.Add<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Query.changed()));
		ASSERT_THAT(IsTrue(Query.count() == 1));

		ASSERT_THAT(IsFalse(Query.changed()));
		
		TestEntity.Modified<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Query.changed()));
		ASSERT_THAT(IsTrue(Query.count() == 1));

		TestEntity.Remove<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Query.changed()));
		ASSERT_THAT(IsTrue(Query.count() == 0));
	}

	TEST_METHOD(BasicComponentModified_Modified_StaticStructAPI)
	{
		TestEntity.Add(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsTrue(Query.changed()));
		ASSERT_THAT(IsTrue(Query.count() == 1));

		ASSERT_THAT(IsFalse(Query.changed()));
		
		TestEntity.Modified(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsTrue(Query.changed()));
		ASSERT_THAT(IsTrue(Query.count() == 1));

		TestEntity.Remove(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsTrue(Query.changed()));
		ASSERT_THAT(IsTrue(Query.count() == 0));
	}

	TEST_METHOD(BasicComponentModified_Modified_EntityAPI)
	{
		TestEntity.Add(ValuedComponentEntity);
		ASSERT_THAT(IsTrue(Query.changed()));
		ASSERT_THAT(IsTrue(Query.count() == 1));

		ASSERT_THAT(IsFalse(Query.changed()));
		
		TestEntity.Modified(ValuedComponentEntity);
		ASSERT_THAT(IsTrue(Query.changed()));
		ASSERT_THAT(IsTrue(Query.count() == 1));

		TestEntity.Remove(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsTrue(Query.changed()));
		ASSERT_THAT(IsTrue(Query.count() == 0));
	}

}; // FlecsComponentModifiedTests


#endif // #if WITH_AUTOMATION_TESTS
