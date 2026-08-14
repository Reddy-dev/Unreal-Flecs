
#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "FlecsCollectonTestTypes.h"

#include "Collections/FlecsCollectionDefinition.h"
#include "Collections/FlecsCollectionWorldSubsystem.h"
#include "Collections/FlecsCollectionEntityRecordFragment.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsCollectionInstantiationTests, "UnrealFlecs.Collections.Instantiation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
			| EAutomationTestFlags::CriticalPriority, "[Flecs]")
{
protected:
	UFlecsCollectionWorldSubsystem* CollectionSubsystem() const
	{
		return UnrealWorld()->GetSubsystemChecked<UFlecsCollectionWorldSubsystem>();
	}

public:
	TEST_METHOD(InstantiateEmptyCollection_CreatesEntityFromPrefab_BuilderAPI)
	{
		const FFlecsEntityHandle CollectionPrefab = CollectionSubsystem()->RegisterCollectionBuilder([](FFlecsCollectionBuilder& Builder)
		{
			Builder
				.Name("TestCollectionToInstantiate");
		});

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddCollection(CollectionPrefab);
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));

		TestEntity.RemoveCollection(CollectionPrefab);
		ASSERT_THAT(IsFalse(TestEntity.HasCollection(CollectionPrefab)));
	}

	TEST_METHOD(InstantiateEmptyCollection_CreatesEntityFromPrefab_DefinitionAPI)
	{
		FFlecsCollectionDefinition Def;
		{
			FFlecsCollectionBuilder Builder = FFlecsCollectionBuilder::Create(Def)
				.Name("TestCollectionToInstantiate_Def");
		}
		
		const FFlecsEntityHandle CollectionPrefab
			= CollectionSubsystem()->RegisterCollectionDefinition(TEXT("TestCollectionToInstantiate_Def"), Def);
		
		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddCollection(CollectionPrefab);
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));

		TestEntity.RemoveCollection(CollectionPrefab);
		ASSERT_THAT(IsFalse(TestEntity.HasCollection(CollectionPrefab)));
	}

	TEST_METHOD(InstantiateCollection_AppliesEnumAndTypedPair_BuilderAPI)
	{
		World()->RegisterComponentType<EFlecsTestEnum_UENUM>();
		World()->RegisterComponentType<FUSTRUCTPairTestComponent>();
		World()->RegisterComponentType<FUSTRUCTPairTestComponent_Second>();

		const FFlecsEntityHandle CollectionPrefab = CollectionSubsystem()->RegisterCollectionBuilder([](FFlecsCollectionBuilder& Builder)
		{
			Builder
				.Name("TestCollectionWithEnumAndPair")
				.Add(EFlecsTestEnum_UENUM::Two)
				.AddPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>();
		});

		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddCollection(CollectionPrefab);
		ASSERT_THAT(IsTrue(TestEntity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Two)));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
	}

	TEST_METHOD(InstantiateEmptyCollection_CreatesEntityFromPrefab_ClassBuilderAPI)
	{
		const FFlecsEntityHandle CollectionPrefab = CollectionSubsystem()->RegisterCollectionClass(UFlecsCollectionTestClassNoInterface::StaticClass(),
			[](FFlecsCollectionBuilder& Builder)
		{
			// No-op
		});

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddCollection(CollectionPrefab);
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(UFlecsCollectionTestClassNoInterface::StaticClass())));
		ASSERT_THAT(IsTrue(TestEntity.HasCollection<UFlecsCollectionTestClassNoInterface>()));

		TestEntity.RemoveCollection(CollectionPrefab);
		ASSERT_THAT(IsFalse(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsFalse(TestEntity.HasCollection(UFlecsCollectionTestClassNoInterface::StaticClass())));
		ASSERT_THAT(IsFalse(TestEntity.HasCollection<UFlecsCollectionTestClassNoInterface>()));
	}

	TEST_METHOD(InstantiateCollection_Inherited_CreatesEntityFromPrefab_ClassInterfaceAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
		
		const FFlecsEntityHandle CollectionPrefab
			= CollectionSubsystem()->RegisterCollectionInterfaceClass(UFlecsCollectionTestClassWithInterface_Inherited::StaticClass());

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddCollection(CollectionPrefab);
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(UFlecsCollectionTestClassWithInterface_Inherited::StaticClass())));
		ASSERT_THAT(IsTrue(TestEntity.HasCollection<UFlecsCollectionTestClassWithInterface_Inherited>()));
		
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));

		TestEntity.RemoveCollection(CollectionPrefab);
		ASSERT_THAT(IsFalse(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsFalse(TestEntity.HasCollection(UFlecsCollectionTestClassWithInterface_Inherited::StaticClass())));
		ASSERT_THAT(IsFalse(TestEntity.HasCollection<UFlecsCollectionTestClassWithInterface_Inherited>()));
		
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));
	}

	TEST_METHOD(InstantiateCollection_Inherited_CreatesEntityFromPrefab_TypedCPPInterfaceAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
		
		const FFlecsEntityHandle CollectionPrefab
			= CollectionSubsystem()->RegisterCollectionInterfaceClass<UFlecsCollectionTestClassWithInterface_Inherited>();

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddCollection(CollectionPrefab);
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(UFlecsCollectionTestClassWithInterface_Inherited::StaticClass())));
		ASSERT_THAT(IsTrue(TestEntity.HasCollection<UFlecsCollectionTestClassWithInterface_Inherited>()));
		
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));

		TestEntity.RemoveCollection(CollectionPrefab);
		ASSERT_THAT(IsFalse(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsFalse(TestEntity.HasCollection(UFlecsCollectionTestClassWithInterface_Inherited::StaticClass())));
		ASSERT_THAT(IsFalse(TestEntity.HasCollection<UFlecsCollectionTestClassWithInterface_Inherited>()));

		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));
	}

	TEST_METHOD(InstantiateCollection_Inherited_CreatesEntityFromPrefab_DefinitionAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
		
		FFlecsCollectionDefinition Def;
		{
			FFlecsCollectionBuilder Builder = FFlecsCollectionBuilder::Create(Def)
				.Name("TestCollectionToInstantiate_Def")
				.Add<FFlecsTestStruct_Tag_Inherited>();
		}
		
		const FFlecsEntityHandle CollectionPrefab
			= CollectionSubsystem()->RegisterCollectionDefinition(TEXT("TestCollectionToInstantiate_Def"), Def);
		
		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddCollection(CollectionPrefab);
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));

		TestEntity.RemoveCollection(CollectionPrefab);
		ASSERT_THAT(IsFalse(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));
	}

	TEST_METHOD(InstantiateCollection_Inherited_CreatesEntityFromPrefab_BuilderAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
		
		const FFlecsEntityHandle CollectionPrefab = CollectionSubsystem()->RegisterCollectionBuilder([](FFlecsCollectionBuilder& Builder)
		{
			Builder
				.Name("TestCollectionToInstantiate")
				.Add<FFlecsTestStruct_Tag_Inherited>();
		});

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddCollection(CollectionPrefab);
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));

		TestEntity.RemoveCollection(CollectionPrefab);
		ASSERT_THAT(IsFalse(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));
	}

	TEST_METHOD(InstantiateCollection_Inherited_CreatesEntityFromPrefab_ClassBuilderAPI)
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

		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddCollection(CollectionPrefab);
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(UFlecsCollectionTestClassNoInterface::StaticClass())));
		ASSERT_THAT(IsTrue(TestEntity.HasCollection<UFlecsCollectionTestClassNoInterface>()));
		
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));
		ASSERT_THAT(IsFalse(TestEntity.Owns<FFlecsTestStruct_Tag_Inherited>()));

		TestEntity.RemoveCollection(CollectionPrefab);
		ASSERT_THAT(IsFalse(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsFalse(TestEntity.HasCollection(UFlecsCollectionTestClassNoInterface::StaticClass())));
		ASSERT_THAT(IsFalse(TestEntity.HasCollection<UFlecsCollectionTestClassNoInterface>()));
		
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));
	}

	TEST_METHOD(InstantiateCollection_Inherited_CreatesEntityFromPrefab_Deferred_ClassInterfaceAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
		
		const FFlecsEntityHandle CollectionPrefab
			= CollectionSubsystem()->RegisterCollectionInterfaceClass(UFlecsCollectionTestClassWithInterface_Inherited::StaticClass());

		ASSERT_THAT(IsTrue(CollectionPrefab.IsValid()));
		ASSERT_THAT(IsTrue(CollectionPrefab.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity("TestEntity");
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		World()->BeginDefer();

			TestEntity.AddCollection(CollectionPrefab);

			ASSERT_THAT(IsFalse(TestEntity.HasCollection(CollectionPrefab)));
			ASSERT_THAT(IsFalse(TestEntity.HasCollection(UFlecsCollectionTestClassWithInterface_Inherited::StaticClass())));
			ASSERT_THAT(IsFalse(TestEntity.HasCollection<UFlecsCollectionTestClassWithInterface_Inherited>()));

			ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));

		World()->EndDefer();

		ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsTrue(TestEntity.HasCollection(UFlecsCollectionTestClassWithInterface_Inherited::StaticClass())));
		ASSERT_THAT(IsTrue(TestEntity.HasCollection<UFlecsCollectionTestClassWithInterface_Inherited>()));
		
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));

		World()->BeginDefer();

			TestEntity.RemoveCollection(CollectionPrefab);

			ASSERT_THAT(IsTrue(TestEntity.HasCollection(CollectionPrefab)));
			ASSERT_THAT(IsTrue(TestEntity.HasCollection(UFlecsCollectionTestClassWithInterface_Inherited::StaticClass())));
			ASSERT_THAT(IsTrue(TestEntity.HasCollection<UFlecsCollectionTestClassWithInterface_Inherited>()));

			ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));

		World()->EndDefer();

		
		ASSERT_THAT(IsFalse(TestEntity.HasCollection(CollectionPrefab)));
		ASSERT_THAT(IsFalse(TestEntity.HasCollection(UFlecsCollectionTestClassWithInterface_Inherited::StaticClass())));
		ASSERT_THAT(IsFalse(TestEntity.HasCollection<UFlecsCollectionTestClassWithInterface_Inherited>()));
		
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Tag_Inherited>()));
	}

}; // FlecsCollectionInstantiationTests

#endif // WITH_AUTOMATION_TESTS
