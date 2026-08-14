// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Worlds/FlecsWorld.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsComponentTagOperationTests,
								 "UnrealFlecs.Components.Operations.Tags",
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
	TEST_METHOD(BasicTagAddRemove_Add_CPPAPI_Remove_CPPAPI)
	{
		TestEntity.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity.Has(TagEntity)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Tag::StaticStruct())));

		TestEntity.Remove<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsFalse(TestEntity.Has(TagEntity)));
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsFalse(TestEntity.Has(FFlecsTestStruct_Tag::StaticStruct())));
	}

	TEST_METHOD(BasicTagAddRemove_Add_StaticStructAPI_Remove_StaticStructAPI)
	{
		TestEntity.Add(FFlecsTestStruct_Tag::StaticStruct());
		ASSERT_THAT(IsTrue(TestEntity.Has(TagEntity)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Tag::StaticStruct())));

		TestEntity.Remove(FFlecsTestStruct_Tag::StaticStruct());
		ASSERT_THAT(IsFalse(TestEntity.Has(TagEntity)));
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsFalse(TestEntity.Has(FFlecsTestStruct_Tag::StaticStruct())));
	}

	TEST_METHOD(BasicTagAddRemove_Add_StaticStructAPI_Remove_CPPAPI)
	{
		TestEntity.Add(FFlecsTestStruct_Tag::StaticStruct());
		ASSERT_THAT(IsTrue(TestEntity.Has(TagEntity)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Tag::StaticStruct())));

		TestEntity.Remove<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsFalse(TestEntity.Has(TagEntity)));
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsFalse(TestEntity.Has(FFlecsTestStruct_Tag::StaticStruct())));
	}

	TEST_METHOD(BasicTagAddRemove_Add_CPPAPI_Remove_StaticStructAPI)
	{
		TestEntity.Add<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TestEntity.Has(TagEntity)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Tag::StaticStruct())));

		TestEntity.Remove(FFlecsTestStruct_Tag::StaticStruct());
		ASSERT_THAT(IsFalse(TestEntity.Has(TagEntity)));
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsFalse(TestEntity.Has(FFlecsTestStruct_Tag::StaticStruct())));
	}

	TEST_METHOD(BasicTagAddRemove_Add_EntityAPI_Remove_StaticStructAPI)
	{
		TestEntity.Add(TagEntity);
		ASSERT_THAT(IsTrue(TestEntity.Has(TagEntity)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Tag::StaticStruct())));

		TestEntity.Remove(FFlecsTestStruct_Tag::StaticStruct());
		ASSERT_THAT(IsFalse(TestEntity.Has(TagEntity)));
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsFalse(TestEntity.Has(FFlecsTestStruct_Tag::StaticStruct())));
	}

	TEST_METHOD(BasicTagAddRemove_Add_EntityAPI_Remove_EntityAPI)
	{
		TestEntity.Add(TagEntity);
		ASSERT_THAT(IsTrue(TestEntity.Has(TagEntity)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Tag::StaticStruct())));

		TestEntity.Remove(FFlecsTestStruct_Tag::StaticStruct());
		ASSERT_THAT(IsFalse(TestEntity.Has(TagEntity)));
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsFalse(TestEntity.Has(FFlecsTestStruct_Tag::StaticStruct())));
	}

}; // FlecsComponentTagOperationTests


#endif // #if WITH_AUTOMATION_TESTS
