
#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "FlecsCollectonTestTypes.h"

#include "Collections/FlecsCollectionDefinition.h"
#include "Collections/FlecsCollectionWorldSubsystem.h"
#include "Collections/FlecsCollectionEntityRecordFragment.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsCollectionSubentityTests, "UnrealFlecs.Collections.Subentities",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
			| EAutomationTestFlags::CriticalPriority, "[Flecs]")
{
protected:
	UFlecsCollectionWorldSubsystem* CollectionSubsystem() const
	{
		return UnrealWorld()->GetSubsystemChecked<UFlecsCollectionWorldSubsystem>();
	}

public:
	TEST_METHOD(InstantiateCollectionWithSubEntities_CreatesEntityWithSubEnitities_BuilderAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag>();
		World()->RegisterComponentType<FFlecsTestStruct_Value>();

		const FFlecsEntityHandle CollectionPrefab = CollectionSubsystem()->RegisterCollectionBuilder([](FFlecsCollectionBuilder& Builder)
		{
			Builder
				.Name("TestCollection_WithSubEntities")
				.BeginSubEntity("SubEntity1")
					.Add<FFlecsTestStruct_Tag>()
					.Add<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ 42 })
				.EndSubEntity();
		});

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity_WithSubEntities");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddCollection(CollectionPrefab);
		
		bool bHasSubEntity = false;
		
		TestEntity.IterateChildren([&](const FFlecsEntityHandle& ChildEntity)
		{
			if (ChildEntity.GetName() == TEXT("SubEntity1"))
			{
				bHasSubEntity = true;
			}
		});
		
		ASSERT_THAT(IsTrue(bHasSubEntity));
		
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsTrue(TestEntity.Lookup<FFlecsEntityView>("SubEntity1").IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Lookup<FFlecsEntityView>("SubEntity1").Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity.Lookup<FFlecsEntityView>("SubEntity1").Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Lookup<FFlecsEntityView>("SubEntity1").Get<FFlecsTestStruct_Value>().Value == 42));
	}

	TEST_METHOD(InstantiateCollectionWithSubEntities_CreatesEntityWithSubEnitities_DefinitionAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag>();
		World()->RegisterComponentType<FFlecsTestStruct_Value>();

		FFlecsCollectionDefinition Def;
		{
			FFlecsCollectionBuilder Builder = FFlecsCollectionBuilder::Create(Def)
				.Name("TestCollection_WithSubEntities_Def")
				.BeginSubEntity("SubEntity1")
					.Add<FFlecsTestStruct_Tag>()
					.Add<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ 123 })
				.EndSubEntity();
		}

		const FFlecsEntityHandle CollectionPrefab
			= CollectionSubsystem()->RegisterCollectionDefinition(TEXT("TestCollection_WithSubEntities_Def"), Def);

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity_WithSubEntities_Def");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddCollection(CollectionPrefab);
		
		bool bHasSubEntity = false;
		TestEntity.IterateChildren([&](const FFlecsEntityHandle& ChildEntity)
		{
			if (ChildEntity.GetName() == TEXT("SubEntity1"))
			{
				bHasSubEntity = true;
			}
		});
		
		ASSERT_THAT(IsTrue(bHasSubEntity));
		
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsTrue(TestEntity.Lookup<FFlecsEntityView>("SubEntity1").IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Lookup<FFlecsEntityView>("SubEntity1").Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity.Lookup<FFlecsEntityView>("SubEntity1").Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Lookup<FFlecsEntityView>("SubEntity1").Get<FFlecsTestStruct_Value>().Value == 123));
	}

	TEST_METHOD(InstantiateCollectionWithSubEntities_CreatesEntityWithSubEnitities_ClassInterfaceAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag>();
		World()->RegisterComponentType<FFlecsTestStruct_Value>();

		const FFlecsEntityHandle CollectionPrefab
			= CollectionSubsystem()->RegisterCollectionInterfaceClass(UFlecsCollectionTestClassWithInterface_WithSubEntities::StaticClass());

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));
		
		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity_WithSubEntities_Class");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		

		TestEntity.AddCollection(CollectionPrefab);
		
		bool bHasSubEntity = false;
		
		TestEntity.IterateChildren([&](const FFlecsEntityHandle& ChildEntity)
		{
			if (ChildEntity.GetName() == TEXT("SubEntity1"))
			{
				bHasSubEntity = true;
			}
		});
		
		ASSERT_THAT(IsTrue(bHasSubEntity));
		
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsTrue(TestEntity.Lookup<FFlecsEntityView>("SubEntity1").IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.Lookup<FFlecsEntityView>("SubEntity1").Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity.Lookup<FFlecsEntityView>("SubEntity1").Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Lookup<FFlecsEntityView>("SubEntity1").Get<FFlecsTestStruct_Value>().Value == 1234));
	}

}; // FlecsCollectionSubentityTests

#endif // WITH_AUTOMATION_TESTS
