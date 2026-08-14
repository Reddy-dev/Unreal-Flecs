// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Worlds/FlecsWorld.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsComponentToggleTests,
								 "UnrealFlecs.Components.Operations.Toggle",
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
	TEST_METHOD(BasicEnableDisableToggleComponent_Enable_CPPAPI_Disable_CPPAPI_Toggle_CPPAPI)
	{
		ASSERT_THAT(IsTrue(ToggleableEntityTest.Has<FFlecsTestStruct_Toggleable>()));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.Has(ToggleableComponentHandle)));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.Has(FFlecsTestStruct_Toggleable::StaticStruct())));
		
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled(ToggleableComponentHandle)));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled(FFlecsTestStruct_Toggleable::StaticStruct())));

		ToggleableEntityTest.Disable<FFlecsTestStruct_Toggleable>();
		ASSERT_THAT(IsFalse(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));

		ToggleableEntityTest.Enable<FFlecsTestStruct_Toggleable>();
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));

		ToggleableEntityTest.Toggle<FFlecsTestStruct_Toggleable>();
		ASSERT_THAT(IsFalse(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));

		ToggleableEntityTest.Toggle<FFlecsTestStruct_Toggleable>();
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));
	}

	TEST_METHOD(BasicEnableDisableToggleComponent_Enable_StaticStructAPI_Disable_StaticStructAPI_Toggle_StaticStructAPI)
	{
		ASSERT_THAT(IsTrue(ToggleableEntityTest.Has<FFlecsTestStruct_Toggleable>()));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.Has(ToggleableComponentHandle)));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.Has(FFlecsTestStruct_Toggleable::StaticStruct())));
		
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled(ToggleableComponentHandle)));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled(FFlecsTestStruct_Toggleable::StaticStruct())));

		ToggleableEntityTest.Disable(FFlecsTestStruct_Toggleable::StaticStruct());
		ASSERT_THAT(IsFalse(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));

		ToggleableEntityTest.Enable(FFlecsTestStruct_Toggleable::StaticStruct());
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));

		ToggleableEntityTest.Toggle(FFlecsTestStruct_Toggleable::StaticStruct());
		ASSERT_THAT(IsFalse(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));

		ToggleableEntityTest.Toggle(FFlecsTestStruct_Toggleable::StaticStruct());
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));
	}

	TEST_METHOD(BasicEnableDisableToggleComponent_Enable_EntityAPI_Disable_EntityAPI_Toggle_EntityAPI)
	{
		ASSERT_THAT(IsTrue(ToggleableEntityTest.Has<FFlecsTestStruct_Toggleable>()));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.Has(ToggleableComponentHandle)));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.Has(FFlecsTestStruct_Toggleable::StaticStruct())));
		
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled(ToggleableComponentHandle)));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled(FFlecsTestStruct_Toggleable::StaticStruct())));

		ToggleableEntityTest.Disable(ToggleableComponentHandle);
		ASSERT_THAT(IsFalse(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));

		ToggleableEntityTest.Enable(ToggleableComponentHandle);
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));

		ToggleableEntityTest.Toggle(ToggleableComponentHandle);
		ASSERT_THAT(IsFalse(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));

		ToggleableEntityTest.Toggle(ToggleableComponentHandle);
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));
	}

	TEST_METHOD(BasicEnableDisableToggleComponent_Enable_CPPAPI_Disable_StaticStructAPI_Toggle_CPPAPI)
	{
		ASSERT_THAT(IsTrue(ToggleableEntityTest.Has<FFlecsTestStruct_Toggleable>()));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.Has(ToggleableComponentHandle)));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.Has(FFlecsTestStruct_Toggleable::StaticStruct())));
		
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled(ToggleableComponentHandle)));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled(FFlecsTestStruct_Toggleable::StaticStruct())));

		ToggleableEntityTest.Disable<FFlecsTestStruct_Toggleable>();
		ASSERT_THAT(IsFalse(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));

		ToggleableEntityTest.Enable(FFlecsTestStruct_Toggleable::StaticStruct());
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));

		ToggleableEntityTest.Toggle<FFlecsTestStruct_Toggleable>();
		ASSERT_THAT(IsFalse(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));

		ToggleableEntityTest.Toggle<FFlecsTestStruct_Toggleable>();
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));
	}

	TEST_METHOD(BasicEnableDisableToggleComponent_Enable_StaticStructAPI_Disable_CPPAPI_Toggle_StaticStructAPI)
	{
		ASSERT_THAT(IsTrue(ToggleableEntityTest.Has<FFlecsTestStruct_Toggleable>()));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.Has(ToggleableComponentHandle)));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.Has(FFlecsTestStruct_Toggleable::StaticStruct())));
		
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled(ToggleableComponentHandle)));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled(FFlecsTestStruct_Toggleable::StaticStruct())));

		ToggleableEntityTest.Disable(FFlecsTestStruct_Toggleable::StaticStruct());
		ASSERT_THAT(IsFalse(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));

		ToggleableEntityTest.Enable<FFlecsTestStruct_Toggleable>();
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));

		ToggleableEntityTest.Toggle(FFlecsTestStruct_Toggleable::StaticStruct());
		ASSERT_THAT(IsFalse(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));

		ToggleableEntityTest.Toggle(FFlecsTestStruct_Toggleable::StaticStruct());
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));
	}

	TEST_METHOD(BasicEnableDisableToggleComponent_Enable_EntityAPI_Disable_CPPAPI_Toggle_EntityAPI)
	{
		ASSERT_THAT(IsTrue(ToggleableEntityTest.Has<FFlecsTestStruct_Toggleable>()));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.Has(ToggleableComponentHandle)));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.Has(FFlecsTestStruct_Toggleable::StaticStruct())));
		
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled(ToggleableComponentHandle)));
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled(FFlecsTestStruct_Toggleable::StaticStruct())));

		ToggleableEntityTest.Disable(ToggleableComponentHandle);
		ASSERT_THAT(IsFalse(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));

		ToggleableEntityTest.Enable(FFlecsTestStruct_Toggleable::StaticStruct());
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));

		ToggleableEntityTest.Toggle(ToggleableComponentHandle);
		ASSERT_THAT(IsFalse(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));

		ToggleableEntityTest.Toggle(ToggleableComponentHandle);
		ASSERT_THAT(IsTrue(ToggleableEntityTest.IsEnabled<FFlecsTestStruct_Toggleable>()));
	}

}; // FlecsComponentToggleTests


#endif // #if WITH_AUTOMATION_TESTS
