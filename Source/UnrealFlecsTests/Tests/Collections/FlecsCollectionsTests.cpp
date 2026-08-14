
#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "FlecsCollectonTestTypes.h"

#include "Collections/FlecsCollectionDefinition.h"
#include "Collections/FlecsCollectionWorldSubsystem.h"
#include "Collections/FlecsCollectionEntityRecordFragment.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsCollectionRegistrationTests, "UnrealFlecs.Collections.Registration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
			| EAutomationTestFlags::CriticalPriority, "[Flecs]")
{
protected:
	UFlecsCollectionWorldSubsystem* CollectionSubsystem() const
	{
		return UnrealWorld()->GetSubsystemChecked<UFlecsCollectionWorldSubsystem>();
	}

public:
	TEST_METHOD(SubsystemBootstrapsAndRegistersScope)
	{
		ASSERT_THAT(IsTrue(IsValid(World())));
		ASSERT_THAT(IsTrue(IsValid(CollectionSubsystem())));
	}

	TEST_METHOD(RegisterEmptyCollection_CreatesPrefab_DefinitionBuilderAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
		
		FFlecsCollectionDefinition Def;
		{
			FFlecsCollectionBuilder Builder = FFlecsCollectionBuilder::Create(Def)
				.Name("TestCollection_Def");
		}
		
		const FFlecsEntityHandle Prefab = CollectionSubsystem()->RegisterCollectionDefinition(TEXT("TestCollection_Def"), Def);
		
		ASSERT_THAT(IsTrue(Prefab.IsValid()));
		ASSERT_THAT(IsTrue(Prefab.Has(flecs::Prefab)));
		ASSERT_THAT(IsTrue(Prefab.GetName() == TEXT("TestCollection_Def")));

		ASSERT_THAT(IsTrue(Prefab.Has<FFlecsCollectionPrefabTag>()));
		ASSERT_THAT(IsFalse(Prefab.Has<FFlecsTestStruct_Tag_Inherited>()));

		ASSERT_THAT(IsTrue(CollectionSubsystem()->IsCollectionRegistered(FFlecsCollectionId::Make("TestCollection_Def"))));
	}

	TEST_METHOD(RegisterEmptyCollection_CreatesPrefab_CPPBuilderAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
		
		const FFlecsEntityHandle Prefab = CollectionSubsystem()->RegisterCollectionBuilder([](FFlecsCollectionBuilder& Builder)
		{
			Builder
				.Name("TestCollection_CPP");
		});

		ASSERT_THAT(IsTrue(Prefab.IsValid()));
		ASSERT_THAT(IsTrue(Prefab.Has(flecs::Prefab)));
		ASSERT_THAT(IsTrue(Prefab.GetName() == TEXT("TestCollection_CPP")));

		ASSERT_THAT(IsTrue(Prefab.Has<FFlecsCollectionPrefabTag>()));
		ASSERT_THAT(IsFalse(Prefab.Has<FFlecsTestStruct_Tag_Inherited>()));

		ASSERT_THAT(IsTrue(CollectionSubsystem()->IsCollectionRegistered(FFlecsCollectionId::Make("TestCollection_CPP"))));
	}

	TEST_METHOD(RegisterEmptyCollection_UClassBuilderAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();

		// Uses the class name as the collection name
		const FFlecsEntityHandle Prefab = CollectionSubsystem()->RegisterCollectionClass(UFlecsCollectionTestClassNoInterface::StaticClass(),
			[](FFlecsCollectionBuilder& Builder)
		{

		});

		ASSERT_THAT(IsTrue(Prefab.IsValid()));
		ASSERT_THAT(IsTrue(Prefab.Has(flecs::Prefab)));
		ASSERT_THAT(IsTrue(Prefab.GetName() == TEXT("UFlecsCollectionTestClassNoInterface")));

		ASSERT_THAT(IsTrue(Prefab.Has<FFlecsCollectionPrefabTag>()));
		ASSERT_THAT(IsFalse(Prefab.Has<FFlecsTestStruct_Tag_Inherited>()));

		ASSERT_THAT(IsTrue(CollectionSubsystem()->IsCollectionRegistered(FFlecsCollectionId::Make("UFlecsCollectionTestClassNoInterface"))));
	}

	TEST_METHOD(RegisterEmptyCollection_CreatesPrefab_ClassBuilderAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();

		// Uses the class name as the collection name
		const FFlecsEntityHandle Prefab = CollectionSubsystem()->RegisterCollectionClass(UFlecsCollectionTestClassNoInterface::StaticClass(),
			[](FFlecsCollectionBuilder& Builder)
		{

		});
		
		ASSERT_THAT(IsTrue(Prefab.IsValid()));
		ASSERT_THAT(IsTrue(Prefab.Has(flecs::Prefab)));
		ASSERT_THAT(IsTrue(Prefab.GetName() == TEXT("UFlecsCollectionTestClassNoInterface")));

		ASSERT_THAT(IsTrue(Prefab.Has<FFlecsCollectionPrefabTag>()));
		ASSERT_THAT(IsFalse(Prefab.Has<FFlecsTestStruct_Tag_Inherited>()));

		ASSERT_THAT(IsTrue(CollectionSubsystem()->IsCollectionRegistered(FFlecsCollectionId::Make("UFlecsCollectionTestClassNoInterface"))));
	}
	
	TEST_METHOD(RegisterCollectionFromDefinition_CreatesPrefabWithTag_DefinitionBuilderAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
		
		FFlecsCollectionDefinition Def;
		{
			FFlecsCollectionBuilder Builder = FFlecsCollectionBuilder::Create(Def)
				.Name("TestCollection_Def")
				.Add<FFlecsTestStruct_Tag_Inherited>();
		}
		
		const FFlecsEntityHandle Prefab = CollectionSubsystem()->RegisterCollectionDefinition(TEXT("TestCollection_Def"), Def);
		ASSERT_THAT(IsTrue(Prefab.IsValid()));
		ASSERT_THAT(IsTrue(Prefab.Has(flecs::Prefab)));
		
		ASSERT_THAT(IsTrue(Prefab.Has<FFlecsCollectionPrefabTag>()));
		ASSERT_THAT(IsTrue(Prefab.Has<FFlecsTestStruct_Tag_Inherited>()));
	}

	TEST_METHOD(RegisterCollectionFromDefinition_CreatesPrefabWithTag_CPPBuilderAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
		
		const FFlecsEntityHandle Prefab = CollectionSubsystem()->RegisterCollectionBuilder([](FFlecsCollectionBuilder& Builder)
		{
			Builder
				.Name("TestCollection_CPP")
				.Add<FFlecsTestStruct_Tag_Inherited>();
		});

		ASSERT_THAT(IsTrue(Prefab.IsValid()));
		ASSERT_THAT(IsTrue(Prefab.Has(flecs::Prefab)));
		
		ASSERT_THAT(IsTrue(Prefab.Has<FFlecsCollectionPrefabTag>()));
		ASSERT_THAT(IsTrue(Prefab.Has<FFlecsTestStruct_Tag_Inherited>()));
	}

	TEST_METHOD(RegisterCollectionFromClass_CreatesPrefabWithTag_ClassBuilderAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();

		// Uses the class name as the collection name
		const FFlecsEntityHandle Prefab = CollectionSubsystem()->RegisterCollectionClass(UFlecsCollectionTestClassNoInterface::StaticClass(),
			[](FFlecsCollectionBuilder& Builder)
		{
			Builder
				.Add<FFlecsTestStruct_Tag_Inherited>();
		});
		
		ASSERT_THAT(IsTrue(Prefab.IsValid()));
		ASSERT_THAT(IsTrue(Prefab.Has(flecs::Prefab)));
		
		ASSERT_THAT(IsTrue(Prefab.Has<FFlecsCollectionPrefabTag>()));
		ASSERT_THAT(IsTrue(Prefab.Has<FFlecsTestStruct_Tag_Inherited>()));
	}

	// Uses the class name as the collection name
	TEST_METHOD(RegisterCollectionFromClass_CreatesPrefabWithTag_UClassInterfaceAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();

		// Uses the class name as the collection name
		const FFlecsEntityHandle Prefab
			= CollectionSubsystem()->RegisterCollectionInterfaceClass(UFlecsCollectionTestClassWithInterface_Inherited::StaticClass());
		
		ASSERT_THAT(IsTrue(Prefab.IsValid()));
		ASSERT_THAT(IsTrue(Prefab.Has(flecs::Prefab)));
		ASSERT_THAT(IsTrue(Prefab.GetName() == TEXT("UFlecsCollectionTestClassWithInterface_Inherited")));
		
		ASSERT_THAT(IsTrue(Prefab.Has<FFlecsCollectionPrefabTag>()));
		ASSERT_THAT(IsTrue(Prefab.Has<FFlecsTestStruct_Tag_Inherited>()));
	}

	// Uses the class name as the collection name
	TEST_METHOD(RegisterCollectionFromClass_CreatesPrefabWithTag_TypedCPPInterfaceAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();

		// Uses the class name as the collection name
		const FFlecsEntityHandle Prefab
			= CollectionSubsystem()->RegisterCollectionInterfaceClass<UFlecsCollectionTestClassWithInterface_Inherited>();
		
		ASSERT_THAT(IsTrue(Prefab.IsValid()));
		ASSERT_THAT(IsTrue(Prefab.Has(flecs::Prefab)));
		ASSERT_THAT(IsTrue(Prefab.GetName() == TEXT("UFlecsCollectionTestClassWithInterface_Inherited")));
		
		ASSERT_THAT(IsTrue(Prefab.Has<FFlecsCollectionPrefabTag>()));
		ASSERT_THAT(IsTrue(Prefab.Has<FFlecsTestStruct_Tag_Inherited>()));
	}

	TEST_METHOD(RegisterCollectionFromDefinition_CreatesPrefabWithSubEntity_DefinitionAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
		
		FFlecsCollectionDefinition Def;
		{
			FFlecsCollectionBuilder Builder = FFlecsCollectionBuilder::Create(Def)
				.Name("TestCollectionWithSubEntities_Def")
				.Add<FFlecsTestStruct_Tag_Inherited>();

			Builder.BeginSubEntity()
				.Add<FFlecsTestStruct_Tag_Inherited>()
			.EndSubEntity();
		}

		const FFlecsEntityHandle Prefab
			= CollectionSubsystem()->RegisterCollectionDefinition(TEXT("TestCollectionWithSubEntities_Def"), Def);

		ASSERT_THAT(IsTrue(Prefab.IsValid()));
		ASSERT_THAT(IsTrue(Prefab.Has(flecs::Prefab)));
		ASSERT_THAT(IsTrue(Prefab.GetName() == TEXT("TestCollectionWithSubEntities_Def")));
		ASSERT_THAT(IsTrue(Prefab.Has<FFlecsCollectionPrefabTag>()));
		
		ASSERT_THAT(IsTrue(Prefab.Has<FFlecsTestStruct_Tag_Inherited>()));

		bool bFoundSubEntity = false;
		Prefab.IterateChildren([&](const FFlecsEntityHandle& ChildEntity)
		{
			if (ChildEntity.Has<FFlecsTestStruct_Tag_Inherited>())
			{
				bFoundSubEntity = true;
			}
		});

		ASSERT_THAT(IsTrue(bFoundSubEntity));
	}

	TEST_METHOD(RegisterCollectionFromDefinition_CreatesPrefabWithSubEntity_WithName_DefinitionAPI)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
		
		FFlecsCollectionDefinition Def;
		{
			FFlecsCollectionBuilder Builder = FFlecsCollectionBuilder::Create(Def)
				.Name("TestCollectionWithSubEntities_Def")
				.Add<FFlecsTestStruct_Tag_Inherited>();

			Builder.BeginSubEntity("SubEntity1")
				.Add<FFlecsTestStruct_Tag_Inherited>()
			.EndSubEntity();
		}

		const FFlecsEntityHandle Prefab
			= CollectionSubsystem()->RegisterCollectionDefinition(TEXT("TestCollectionWithSubEntities_Def"), Def);

		ASSERT_THAT(IsTrue(Prefab.IsValid()));
		ASSERT_THAT(IsTrue(Prefab.Has(flecs::Prefab)));
		ASSERT_THAT(IsTrue(Prefab.GetName() == TEXT("TestCollectionWithSubEntities_Def")));
		ASSERT_THAT(IsTrue(Prefab.Has<FFlecsCollectionPrefabTag>()));
		
		ASSERT_THAT(IsTrue(Prefab.Has<FFlecsTestStruct_Tag_Inherited>()));

		const FFlecsEntityView SubEntity1View = Prefab.Lookup<FFlecsEntityView>("SubEntity1");
		ASSERT_THAT(IsTrue(SubEntity1View.IsValid()));
		ASSERT_THAT(IsTrue(SubEntity1View.Has<FFlecsTestStruct_Tag_Inherited>()));
	}

}; // FlecsCollectionRegistrationTests

#endif // WITH_AUTOMATION_TESTS
