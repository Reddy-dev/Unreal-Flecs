
#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "FlecsCollectonTestTypes.h"

#include "Collections/FlecsCollectionDefinition.h"
#include "Collections/FlecsCollectionWorldSubsystem.h"
#include "Collections/FlecsCollectionEntityRecordFragment.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsCollectionRecordTests, "UnrealFlecs.Collections.EntityRecords",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
			| EAutomationTestFlags::CriticalPriority, "[Flecs]")
{
protected:
	UFlecsCollectionWorldSubsystem* CollectionSubsystem() const
	{
		return UnrealWorld()->GetSubsystemChecked<UFlecsCollectionWorldSubsystem>();
	}

public:
	TEST_METHOD(InstantiateCollectionWithinEntityRecord_CreatesEntityWithCollection_BuilderAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
	
		const FFlecsEntityHandle CollectionPrefab = CollectionSubsystem()->RegisterCollectionBuilder([](FFlecsCollectionBuilder& Builder)
		{
			Builder
				.Name("TestCollection_WithinEntityRecord")
				.Add<FFlecsTestStruct_Tag_Inherited>();
		});

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		FFlecsCollectionInstancedReference CollectionRef(FFlecsCollectionReference::FromId("TestCollection_WithinEntityRecord"));

		FFlecsEntityRecord EntityRecord = FFlecsEntityRecord();
		EntityRecord.AddFragment<FFlecsCollectionsEntityRecordFragment>(FFlecsCollectionsEntityRecordFragment({CollectionRef}));
		ASSERT_THAT(IsTrue(EntityRecord.HasFragment<FFlecsCollectionsEntityRecordFragment>()));
		
		const FFlecsEntityHandle TestEntity = World()->CreateEntityWithRecord(EntityRecord, "TestEntity_WithCollectionInRecord");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));
		ASSERT_THAT(IsTrue(TestEntity.GetName() == TEXT("TestEntity_WithCollectionInRecord")));
	}

	TEST_METHOD(InstantiateCollectionWithinEntityRecord_CreatesEntityWithCollection_DefinitionAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
	
		FFlecsCollectionDefinition Def;
		{
			FFlecsCollectionBuilder Builder = FFlecsCollectionBuilder::Create(Def)
				.Name("TestCollection_WithinEntityRecord_Def")
				.Add<FFlecsTestStruct_Tag_Inherited>();
		}
		
		const FFlecsEntityHandle CollectionPrefab
			= CollectionSubsystem()->RegisterCollectionDefinition(TEXT("TestCollection_WithinEntityRecord_Def"), Def);

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		FFlecsCollectionInstancedReference CollectionRef(FFlecsCollectionReference::FromId("TestCollection_WithinEntityRecord_Def"));

		FFlecsEntityRecord EntityRecord = FFlecsEntityRecord();
		EntityRecord.AddFragment<FFlecsCollectionsEntityRecordFragment>(FFlecsCollectionsEntityRecordFragment({CollectionRef}));
		ASSERT_THAT(IsTrue(EntityRecord.HasFragment<FFlecsCollectionsEntityRecordFragment>()));
		
		const FFlecsEntityHandle TestEntity = World()->CreateEntityWithRecord(EntityRecord, "TestEntity_WithCollectionInRecord_Def");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));
		ASSERT_THAT(IsTrue(TestEntity.GetName() == TEXT("TestEntity_WithCollectionInRecord_Def")));
	}

	TEST_METHOD(InstantiateCollectionWithinEntityRecord_CreatesEntityWithCollection_ClassBuilderAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
	
		const FFlecsEntityHandle CollectionPrefab = CollectionSubsystem()->RegisterCollectionClass(UFlecsCollectionTestClassNoInterface::StaticClass(),
			[](FFlecsCollectionBuilder& Builder)
				{
					Builder
						.Add<FFlecsTestStruct_Tag_Inherited>();
				});

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		FFlecsCollectionInstancedReference CollectionRef(FFlecsCollectionReference::FromClass(UFlecsCollectionTestClassNoInterface::StaticClass()));

		FFlecsEntityRecord EntityRecord = FFlecsEntityRecord();
		EntityRecord.AddFragment<FFlecsCollectionsEntityRecordFragment>(FFlecsCollectionsEntityRecordFragment({CollectionRef}));
		ASSERT_THAT(IsTrue(EntityRecord.HasFragment<FFlecsCollectionsEntityRecordFragment>()));
		
		const FFlecsEntityHandle TestEntity = World()->CreateEntityWithRecord(EntityRecord, "TestEntity_WithCollectionInRecord_Class");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));
		ASSERT_THAT(IsTrue(TestEntity.GetName() == TEXT("TestEntity_WithCollectionInRecord_Class")));
	}

	TEST_METHOD(InstantiateCollectionWithinEntityRecord_CreatesEntityWithCollection_ClassInterfaceAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
	
		const FFlecsEntityHandle CollectionPrefab
			= CollectionSubsystem()->RegisterCollectionInterfaceClass(UFlecsCollectionTestClassWithInterface_Inherited::StaticClass());

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		FFlecsCollectionInstancedReference CollectionRef(FFlecsCollectionReference::FromClass(UFlecsCollectionTestClassWithInterface_Inherited::StaticClass()));

		FFlecsEntityRecord EntityRecord = FFlecsEntityRecord();
		EntityRecord.AddFragment<FFlecsCollectionsEntityRecordFragment>(FFlecsCollectionsEntityRecordFragment({CollectionRef}));
		ASSERT_THAT(IsTrue(EntityRecord.HasFragment<FFlecsCollectionsEntityRecordFragment>()));
		
		const FFlecsEntityHandle TestEntity = World()->CreateEntityWithRecord(EntityRecord, "TestEntity_WithCollectionInRecord_ClassInterface");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));
		ASSERT_THAT(IsTrue(TestEntity.GetName() == TEXT("TestEntity_WithCollectionInRecord_ClassInterface")));
	}

	TEST_METHOD(InstantiateCollectionWithinEntityRecord_CreatesEntityWithCollectionWithValue_ClassInterfaceAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag>();
		World()->RegisterComponentType<FFlecsTestStruct_Value>();
	
		const FFlecsEntityHandle CollectionPrefab
			= CollectionSubsystem()->RegisterCollectionInterfaceClass(UFlecsCollectionTestClassWithInterface_Parameterized::StaticClass());

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		FFlecsCollectionInstancedReference CollectionRef(FFlecsCollectionReference::FromClass(UFlecsCollectionTestClassWithInterface_Parameterized::StaticClass()));

		FFlecsEntityRecord EntityRecord = FFlecsEntityRecord();
		EntityRecord.AddFragment<FFlecsCollectionsEntityRecordFragment>(FFlecsCollectionsEntityRecordFragment({CollectionRef}));
		ASSERT_THAT(IsTrue(EntityRecord.HasFragment<FFlecsCollectionsEntityRecordFragment>()));
		
		const FFlecsEntityHandle TestEntity = World()->CreateEntityWithRecord(EntityRecord, "TestEntity_WithCollectionInRecord_ClassInterface_Parameterized");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Get<FFlecsTestStruct_Value>().Value == 33));
	}

	TEST_METHOD(InstantiateCollectionWithinEntityRecord_CreatesEntityWithCollectionWithValueAndExplicitParams_ClassInterfaceAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag>();
		World()->RegisterComponentType<FFlecsTestStruct_Value>();
	
		const FFlecsEntityHandle CollectionPrefab
			= CollectionSubsystem()->RegisterCollectionInterfaceClass(UFlecsCollectionTestClassWithInterface_Parameterized::StaticClass());

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		FFlecsCollectionInstancedReference CollectionRef(FFlecsCollectionReference::FromClass(UFlecsCollectionTestClassWithInterface_Parameterized::StaticClass()),
			FInstancedStruct::Make<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ 42 }));

		FFlecsEntityRecord EntityRecord = FFlecsEntityRecord();
		EntityRecord.AddFragment<FFlecsCollectionsEntityRecordFragment>(FFlecsCollectionsEntityRecordFragment({CollectionRef}));
		ASSERT_THAT(IsTrue(EntityRecord.HasFragment<FFlecsCollectionsEntityRecordFragment>()));
		
		const FFlecsEntityHandle TestEntity = World()->CreateEntityWithRecord(EntityRecord, "TestEntity_WithCollectionInRecord_ClassInterface_Parameterized_Explicit");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Get<FFlecsTestStruct_Value>().Value == 42));
	}
	
	TEST_METHOD(InstantiateMultipleCollectionsWithinEntityRecord_CreatesEntityWithBothCollections_BuilderAndDefinitionAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
		World()->RegisterComponentType<FFlecsTestStruct_Tag>();
		World()->RegisterComponentType<FFlecsTestStruct_Value>();
		
		const FFlecsEntityHandle CollectionPrefabA = CollectionSubsystem()->RegisterCollectionBuilder([](FFlecsCollectionBuilder& Builder)
		{
			Builder
				.Name("TestCollection_Multi_A_Builder")
				.Add<FFlecsTestStruct_Tag_Inherited>();
		});

		ASSERT_THAT(IsTrue(CollectionPrefabA.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefabA.Has(flecs::Prefab)));
		
		FFlecsCollectionDefinition DefB;
		{
			FFlecsCollectionBuilder Builder = FFlecsCollectionBuilder::Create(DefB)
				.Name("TestCollection_Multi_B_Definition")
				.Add<FFlecsTestStruct_Tag>()
				.Add<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ 77 });
		}

		const FFlecsEntityHandle CollectionPrefabB
			= CollectionSubsystem()->RegisterCollectionDefinition(TEXT("TestCollection_Multi_B_Definition"), DefB);

		ASSERT_THAT(IsTrue(CollectionPrefabB.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefabB.Has(flecs::Prefab)));
		
		const FFlecsCollectionInstancedReference RefA(FFlecsCollectionReference::FromId("TestCollection_Multi_A_Builder"));
		const FFlecsCollectionInstancedReference RefB(FFlecsCollectionReference::FromId("TestCollection_Multi_B_Definition"));

		FFlecsEntityRecord EntityRecord;
		EntityRecord.AddFragment<FFlecsCollectionsEntityRecordFragment>(
			FFlecsCollectionsEntityRecordFragment({ RefA, RefB })
		);

		ASSERT_THAT(IsTrue(EntityRecord.HasFragment<FFlecsCollectionsEntityRecordFragment>()));
		
		const FFlecsEntityHandle TestEntity = World()->CreateEntityWithRecord(EntityRecord, "TestEntity_WithMultipleCollections_Record");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefabA)));
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefabB)));

		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Get<FFlecsTestStruct_Value>().Value == 77));
	}
	
	TEST_METHOD(InstantiateMultipleCollectionsWithinEntityRecord_CreatesEntityWithBothCollections_ClassInterfaceAPI_ExplicitParams)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
		World()->RegisterComponentType<FFlecsTestStruct_Tag>();
		World()->RegisterComponentType<FFlecsTestStruct_Value>();

		const FFlecsEntityHandle CollectionPrefabA
			= CollectionSubsystem()->RegisterCollectionInterfaceClass(UFlecsCollectionTestClassWithInterface_Inherited::StaticClass());

		const FFlecsEntityHandle CollectionPrefabB
			= CollectionSubsystem()->RegisterCollectionInterfaceClass(UFlecsCollectionTestClassWithInterface_Parameterized::StaticClass());

		ASSERT_THAT(IsTrue(CollectionPrefabA.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefabA.Has(flecs::Prefab)));
		ASSERT_THAT(IsTrue(CollectionPrefabB.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefabB.Has(flecs::Prefab)));

		const FFlecsCollectionInstancedReference RefA(
			FFlecsCollectionReference::FromClass(UFlecsCollectionTestClassWithInterface_Inherited::StaticClass())
		);
		
		const FFlecsCollectionInstancedReference RefB(
			FFlecsCollectionReference::FromClass(UFlecsCollectionTestClassWithInterface_Parameterized::StaticClass()),
			FInstancedStruct::Make<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ 123 })
		);

		FFlecsEntityRecord EntityRecord;
		EntityRecord.AddFragment<FFlecsCollectionsEntityRecordFragment>(
			FFlecsCollectionsEntityRecordFragment({ RefA, RefB })
		);

		const FFlecsEntityHandle TestEntity = World()->CreateEntityWithRecord(EntityRecord, "TestEntity_WithMultipleCollections_Record_ClassInterface_Explicit");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefabA)));
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefabB)));
		
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));
		
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Get<FFlecsTestStruct_Value>().Value == 123));
	}
	
}; // FlecsCollectionRecordTests

#endif // WITH_AUTOMATION_TESTS
