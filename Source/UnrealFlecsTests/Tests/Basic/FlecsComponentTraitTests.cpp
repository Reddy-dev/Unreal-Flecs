// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Worlds/FlecsWorld.h"

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsComponentTraitTests,
								   "UnrealFlecs.Components.TraitsDependencies",
							   EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
								| EAutomationTestFlags::CriticalPriority,
							   "[Flecs][Entity][Tag][Component][Registration]")
{
	TEST_METHOD(GetComponentPropertyTraits_CPPAPI)
	{
		const FFlecsEntityHandle StructEntity = World()->RegisterComponentType<FFlecsTestStruct_WithPropertyTraits>();
		ASSERT_THAT(IsTrue(StructEntity.IsValid()));

		ASSERT_THAT(IsTrue(StructEntity.Has(flecs::Trait)));
	}

	TEST_METHOD(GetComponentPropertyTraits_StaticStructAPI)
	{
		const FFlecsEntityHandle StaticStructEntity = World()->RegisterComponentType(FFlecsTestStruct_WithPropertyTraits::StaticStruct());
		ASSERT_THAT(IsTrue(StaticStructEntity.IsValid()));

		ASSERT_THAT(IsTrue(StaticStructEntity.Has(flecs::Trait)));
	}

	TEST_METHOD(GetComponentPropertyTraits_CPPOnlyType_CPPAPI)
	{
		const FFlecsEntityHandle StructEntity = World()->RegisterComponentType<FFlecsTest_CPPStruct_Traits>();
		ASSERT_THAT(IsTrue(StructEntity.IsValid()));

		ASSERT_THAT(IsTrue(StructEntity.Has(flecs::Trait)));
	}

	TEST_METHOD(GetComponentWithEmptyRegistrationFunctionNoTraits_StaticStructAPI)
	{
		const FFlecsEntityHandle StaticStructEntity = World()->RegisterComponentType(FFlecsTestStruct_EmptyRegistrationFunction::StaticStruct());
		ASSERT_THAT(IsTrue(StaticStructEntity.IsValid()));

		ASSERT_THAT(IsFalse(StaticStructEntity.Has(flecs::Trait)));
	}

	TEST_METHOD(GetComponentWithEmptyRegistrationFunctionNoTraits_CPPAPI)
	{
		const FFlecsEntityHandle StructEntity = World()->RegisterComponentType<FFlecsTestStruct_EmptyRegistrationFunction>();
		ASSERT_THAT(IsTrue(StructEntity.IsValid()));

		ASSERT_THAT(IsFalse(StructEntity.Has(flecs::Trait)));
	}

	TEST_METHOD(GetComponentWithNoRegistrationLambdaNoTraits_StaticStructAPI)
	{
		const FFlecsEntityHandle StaticStructEntity = World()->RegisterComponentType(FFlecsTestStruct_NoRegistrationLambda::StaticStruct());
		ASSERT_THAT(IsTrue(StaticStructEntity.IsValid()));

		ASSERT_THAT(IsFalse(StaticStructEntity.Has(flecs::Trait)));
	}

	TEST_METHOD(GetComponentWithNoRegistrationLambdaNoTraits_CPPAPI)
	{
		const FFlecsEntityHandle StructEntity = World()->RegisterComponentType<FFlecsTestStruct_NoRegistrationLambda>();
		ASSERT_THAT(IsTrue(StructEntity.IsValid()));

		ASSERT_THAT(IsFalse(StructEntity.Has(flecs::Trait)));
	}
	
	TEST_METHOD(GetComponentPropertyTraitsWithTypedComponentHandleLambda_CPPOnlyType_CPPAPI)
	{
		const FFlecsEntityHandle StructEntity = World()->RegisterComponentType<FFlecsTest_CPPStructValue_Traits_WithTypedComponentHandleInLambda>();
		ASSERT_THAT(IsTrue(StructEntity.IsValid()));
		
		ASSERT_THAT(IsTrue(StructEntity.Has(flecs::Trait)));
		ASSERT_THAT(IsTrue(StructEntity.Has(flecs::PairIsTag)));
	}

	TEST_METHOD(WithTypes_CPPAPI)
	{
		const FFlecsComponentPropertiesDefinition ComponentProperties = FFlecsComponentPropertiesDefinition::Make<FFlecsTestStruct_WithTypes>();
		ASSERT_THAT(IsTrue(ComponentProperties.WithTypes.Num() == 2));

		const FFlecsComponentHandle RequirementA = World()->RegisterComponentType<FFlecsTestStruct_WithTypeRequirementA>();
		const FFlecsComponentHandle RequirementB = World()->RegisterComponentType<FFlecsTestStruct_WithTypeRequirementB>();
		const FFlecsComponentHandle Component = World()->RegisterComponentType<FFlecsTestStruct_WithTypes>();

		ASSERT_THAT(IsTrue(Component.HasPair(flecs::With, RequirementA.GetFlecsId())));
		ASSERT_THAT(IsTrue(Component.HasPair(flecs::With, RequirementB.GetFlecsId())));
	}

	TEST_METHOD(WithTypes_StaticStructAPI)
	{
		const FFlecsEntityHandle RequirementA = World()->RegisterComponentType(FFlecsTestStruct_WithTypeRequirementA::StaticStruct());
		const FFlecsEntityHandle RequirementB = World()->RegisterComponentType(FFlecsTestStruct_WithTypeRequirementB::StaticStruct());
		const FFlecsEntityHandle Component = World()->RegisterComponentType(FFlecsTestStruct_WithTypes::StaticStruct());

		ASSERT_THAT(IsTrue(Component.HasPair(flecs::With, RequirementA.GetFlecsId())));
		ASSERT_THAT(IsTrue(Component.HasPair(flecs::With, RequirementB.GetFlecsId())));
	}
	
}; // FlecsComponentTraitTests


#endif // #if WITH_AUTOMATION_TESTS
