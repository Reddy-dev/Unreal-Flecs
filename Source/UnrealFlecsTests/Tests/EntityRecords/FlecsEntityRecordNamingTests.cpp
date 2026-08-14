// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Components/FlecsSubEntityRecordNameComponent.h"
#include "EntityRecords/FlecsEntityRecord.h"
#include "EntityRecords/FlecsNamedEntityRecordFragment.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsEntityRecordNamingTests, "UnrealFlecs.EntityRecords.NamingSubentities",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
			| EAutomationTestFlags::CriticalPriority, "[Flecs]")
{
protected:
	virtual void OnRegisteredWorldSetUp() override
	{
		World()->RegisterComponentType<EFlecsTestEnum_UENUM>();
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
		World()->RegisterComponentType<FFlecsTestStruct_WithPropertyTraits>();
	}

public:
	TEST_METHOD(CreateEntityWithRecord_WithNamedEntityRecordFragment)
	{
		FFlecsEntityRecord Record;
		Record.AddFragment<FFlecsNamedEntityRecordFragment>("TestEntityWithRecordFragment");
		Record.AddComponent<FFlecsTestStruct_Tag>();

		const FFlecsEntityHandle Entity = World()->CreateEntityWithRecord(Record);
		ASSERT_THAT(IsTrue(Entity.IsValid()));
		ASSERT_THAT(IsTrue(Entity.HasName()));
		
		ASSERT_THAT(AreEqual(TEXT("TestEntityWithRecordFragment"), Entity.GetName()));
		
		ASSERT_THAT(IsTrue(Entity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(Entity.Has(FFlecsTestStruct_Tag::StaticStruct())));
	}
	
	TEST_METHOD(CreatePrefabWithRecord_WithNamedEntityRecordFragment_InheritDontFragmentSubEntityNames)
	{
		FFlecsEntityRecord SubRecord;
		SubRecord.AddFragment<FFlecsNamedEntityRecordFragment>("SubEntityName");
		SubRecord.AddComponent<FFlecsTestStruct_Tag>();
		
		ASSERT_THAT(IsTrue(SubRecord.GetFragment<FFlecsNamedEntityRecordFragment>(0).bNameInheritedSubEntities));

		FFlecsEntityRecord Record;
		Record.AddFragment<FFlecsNamedEntityRecordFragment>("ParentEntityName");
		Record.AddSubEntity(SubRecord);

		const FFlecsEntityHandle PrefabEntity = World()->CreatePrefabWithRecord(Record);
		ASSERT_THAT(IsTrue(PrefabEntity.IsValid()));
		ASSERT_THAT(IsTrue(PrefabEntity.Has(flecs::Prefab)));
		ASSERT_THAT(IsTrue(PrefabEntity.HasName()));
		ASSERT_THAT(AreEqual(TEXT("ParentEntityName"), PrefabEntity.GetName()));
		
		const FFlecsEntityView SubEntityView = PrefabEntity.Lookup<FFlecsEntityView>(TEXT("SubEntityName"));
		ASSERT_THAT(IsTrue(SubEntityView.IsValid()));
		ASSERT_THAT(IsTrue(SubEntityView.HasName()));
		ASSERT_THAT(AreEqual(TEXT("SubEntityName"), SubEntityView.GetName()));
		ASSERT_THAT(IsTrue(SubEntityView.Has<FFlecsSubEntityRecordNameComponent>()));
		ASSERT_THAT(AreEqual(TEXT("SubEntityName"),
			SubEntityView.Get<FFlecsSubEntityRecordNameComponent>().SubEntityName));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		
		ASSERT_THAT(IsTrue(TestEntity.HasName()));
		ASSERT_THAT(AreEqual(TEXT("TestEntity"), TestEntity.GetName()));

		TestEntity.AddPrefab(PrefabEntity);
		
		ASSERT_THAT(IsTrue(TestEntity.HasName()));
		ASSERT_THAT(AreEqual(TEXT("TestEntity"), TestEntity.GetName()));
		
		ASSERT_THAT(IsTrue(TestEntity.IsA(PrefabEntity)));
		
		bool bFoundSubEntity = false;
		TestEntity.IterateChildren([&](const FFlecsEntityHandle& ChildEntity)
		{
			if (ChildEntity.Has<FFlecsTestStruct_Tag>() && ChildEntity.Has<FFlecsSubEntityRecordNameComponent>())
			{
				bFoundSubEntity = true;
			}
		});
		
		ASSERT_THAT(IsTrue(bFoundSubEntity));
		
		ASSERT_THAT(IsTrue(TestEntity.Lookup<FFlecsEntityView>(TEXT("SubEntityName")).IsValid()));
	}
	
	TEST_METHOD(CreatePrefabWithRecord_WithNamedEntityRecordFragment_DontInheritDontFragmentSubEntityNames)
	{
		FFlecsEntityRecord SubRecord;
		SubRecord.AddFragment<FFlecsNamedEntityRecordFragment>("SubEntityName", false);
		SubRecord.AddComponent<FFlecsTestStruct_Tag>();
		
		ASSERT_THAT(IsTrue(!SubRecord.GetFragment<FFlecsNamedEntityRecordFragment>(0).bNameInheritedSubEntities));

		FFlecsEntityRecord Record;
		Record.AddFragment<FFlecsNamedEntityRecordFragment>("ParentEntityName");
		Record.AddSubEntity(SubRecord);

		const FFlecsEntityHandle PrefabEntity = World()->CreatePrefabWithRecord(Record);
		ASSERT_THAT(IsTrue(PrefabEntity.IsValid()));
		ASSERT_THAT(IsTrue(PrefabEntity.Has(flecs::Prefab)));
		ASSERT_THAT(IsTrue(PrefabEntity.HasName()));
		ASSERT_THAT(AreEqual(TEXT("ParentEntityName"), PrefabEntity.GetName()));
		ASSERT_THAT(IsTrue(PrefabEntity.Lookup<FFlecsEntityView>(TEXT("SubEntityName")).IsValid()));
		
		const FFlecsEntityView SubEntityView = PrefabEntity.Lookup<FFlecsEntityView>(TEXT("SubEntityName"));
		ASSERT_THAT(IsTrue(SubEntityView.IsValid()));
		ASSERT_THAT(IsTrue(SubEntityView.HasName()));
		ASSERT_THAT(AreEqual(TEXT("SubEntityName"), SubEntityView.GetName()));
		ASSERT_THAT(IsTrue(!SubEntityView.Has<FFlecsSubEntityRecordNameComponent>()));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		
		ASSERT_THAT(IsTrue(TestEntity.HasName()));
		ASSERT_THAT(AreEqual(TEXT("TestEntity"), TestEntity.GetName()));

		TestEntity.AddPrefab(PrefabEntity);
		
		ASSERT_THAT(IsTrue(TestEntity.HasName()));
		ASSERT_THAT(AreEqual(TEXT("TestEntity"), TestEntity.GetName()));
		
		ASSERT_THAT(IsTrue(TestEntity.IsA(PrefabEntity)));
		
		bool bFoundSubEntity = false;
		TestEntity.IterateChildren([&](const FFlecsEntityHandle& ChildEntity)
		{
			if (ChildEntity.Has<FFlecsTestStruct_Tag>())
			{
				bFoundSubEntity = true;
			}
		});
		
		ASSERT_THAT(IsTrue(bFoundSubEntity));
		
		ASSERT_THAT(IsTrue(!TestEntity.Lookup<FFlecsEntityView>(TEXT("SubEntityName")).IsValid()));
	}
	
}; // FlecsEntityRecordNamingTests

#endif // WITH_AUTOMATION_TESTS
