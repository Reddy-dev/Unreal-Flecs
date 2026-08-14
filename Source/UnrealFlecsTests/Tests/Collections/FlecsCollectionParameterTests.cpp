
#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "FlecsCollectonTestTypes.h"

#include "Collections/FlecsCollectionDefinition.h"
#include "Collections/FlecsCollectionWorldSubsystem.h"
#include "Collections/FlecsCollectionEntityRecordFragment.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsCollectionParameterTests, "UnrealFlecs.Collections.Parameters",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
			| EAutomationTestFlags::CriticalPriority, "[Flecs]")
{
protected:
	UFlecsCollectionWorldSubsystem* CollectionSubsystem() const
	{
		return UnrealWorld()->GetSubsystemChecked<UFlecsCollectionWorldSubsystem>();
	}

public:
	TEST_METHOD(InstantiateParameterizedCollection_CPPBuilderAPI_WithDefaultParameters_DefaultValue_InstancedStructAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag>();
		World()->RegisterComponentType<FFlecsTestStruct_Value>();

		const FFlecsEntityHandle CollectionPrefab = CollectionSubsystem()->RegisterCollectionBuilder([](FFlecsCollectionBuilder& Builder)
		{
			Builder
				.Name("TestCollection_Parameterized")
				.Add<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ 33 })
				.Add<FFlecsTestStruct_Tag>()
				.Parameters(FInstancedStruct::Make<FFlecsTestStruct_Value>({ 33 }),
					[](const FFlecsEntityHandle& InCollectionEntity, const FInstancedStruct& InParams)
				{
					InCollectionEntity.Assign<FFlecsTestStruct_Value>(InParams.Get<FFlecsTestStruct_Value>());
				});
		});

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity_Parameterized_Default");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddCollection(CollectionPrefab);

		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));

		// Verify the value was set from the default params
		const FFlecsTestStruct_Value& Applied = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Applied.Value == 33));

		// Cleanup
	}

	TEST_METHOD(InstantiateParameterizedCollection_CPPBuilderAPI_WithExplicitParameters_AppliesValue_InstancedStructAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag>();
		World()->RegisterComponentType<FFlecsTestStruct_Value>();

		const FFlecsEntityHandle CollectionPrefab = CollectionSubsystem()->RegisterCollectionBuilder([](FFlecsCollectionBuilder& Builder)
		{
			Builder
				.Name("TestCollection_Parameterized")
				.Add<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ 33 })
				.Add<FFlecsTestStruct_Tag>()
				.Parameters<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ 33 },
					[](const FFlecsEntityHandle& InCollectionEntity, const FFlecsTestStruct_Value& InParams)
				{
					InCollectionEntity.Assign<FFlecsTestStruct_Value>(InParams);
				});
		});

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity_Parameterized_Explicit");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddCollection(CollectionPrefab, FInstancedStruct::Make<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{99}));

		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));

		// Verify the value was set from the explicit params
		const FFlecsTestStruct_Value& Applied = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Applied.Value == 99));

		// Cleanup
	}

	TEST_METHOD(InstantiateParameterizedCollection_ClassBuilderAPI_WithExplicitParameters_DefaultValue_InstancedStructAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag>();
		World()->RegisterComponentType<FFlecsTestStruct_Value>();

		const FFlecsEntityHandle CollectionPrefab
			= CollectionSubsystem()->RegisterCollectionInterfaceClass(UFlecsCollectionTestClassWithInterface_Parameterized::StaticClass());

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity_Parameterized_Explicit");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddCollection(CollectionPrefab);

		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));

		// Verify the value was set from the explicit params
		const FFlecsTestStruct_Value& Applied = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Applied.Value == 33));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));

		// Cleanup
	}

	TEST_METHOD(InstantiateParameterizedCollection_ClassBuilderAPI_WithExplicitParameters_AppliesValue_InstancedStructAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag>();
		World()->RegisterComponentType<FFlecsTestStruct_Value>();

		const FFlecsEntityHandle CollectionPrefab
			= CollectionSubsystem()->RegisterCollectionInterfaceClass(UFlecsCollectionTestClassWithInterface_Parameterized::StaticClass());

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity_Parameterized_Explicit");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddCollection(CollectionPrefab, FInstancedStruct::Make<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{99}));

		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));

		// Verify the value was set from the explicit params
		const FFlecsTestStruct_Value& Applied = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Applied.Value == 99));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));

		// Cleanup
	}

}; // FlecsCollectionParameterTests

#endif // WITH_AUTOMATION_TESTS
