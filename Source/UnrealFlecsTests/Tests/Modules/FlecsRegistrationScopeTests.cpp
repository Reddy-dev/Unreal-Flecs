// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "UnrealFlecsTests/Fixtures/FlecsWorldFixture.h"
#include "UnrealFlecsTests/Tests/Types/FlecsRegistrationScopeTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Components/UnrealFlecsPluginTag.h"
#include "General/UnrealFlecsRegistrationScopeType.h"
#include "Properties/FlecsComponentProperties.h"
#include "Worlds/FlecsWorld.h"

namespace UE::Flecs::Tests::RegistrationScope
{
	static FFlecsEntityHandle CreateModuleScope(const TSolidNotNull<UFlecsWorld*> InWorld,
		const FName InScopeName)
	{
		return InWorld->CreateEntity(InScopeName.ToString())
			.Add(flecs::Module);
	}

} // namespace UE::Flecs::Tests::RegistrationScope

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsRegistrationScopeTests,
	"UnrealFlecs.Registration.Scope",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
		| EAutomationTestFlags::CriticalPriority,
	"[Flecs][Registration][Scope]")
{
	TEST_METHOD(ComponentTraits_PreserveSpecializedScopeTypeAndExplicitName)
	{
		const FFlecsComponentPropertiesDefinition ComponentProperties
			= FFlecsComponentPropertiesDefinition::Make<FFlecsRegistrationScopePluginComponent>();

		ASSERT_THAT(IsTrue(ComponentProperties.RegistrationScopeType == EUnrealFlecsRegistrationScopeType::Plugin));
		
		ASSERT_THAT(IsTrue(ComponentProperties.RegistrationScopeName
			== UE::Flecs::Tests::RegistrationScope::PluginScopeName));
	}

	TEST_METHOD(ResolveScopeTypeName_InfersModuleAndPluginNames)
	{
		const UFlecsExplicitModuleRegistrationScopeTestObject* Object
			= GetDefault<UFlecsExplicitModuleRegistrationScopeTestObject>();

		const FName ModuleName = UE::Flecs::Registration::ResolveScopeTypeName(Object,
			EUnrealFlecsRegistrationScopeType::Module);
		const FName PluginName = UE::Flecs::Registration::ResolveScopeTypeName(Object,
			EUnrealFlecsRegistrationScopeType::Plugin);

		ASSERT_THAT(IsTrue(ModuleName == FName(TEXT("UnrealFlecsTests"))));
		ASSERT_THAT(IsTrue(PluginName == FName(TEXT("UnrealFlecs"))));
		ASSERT_THAT(IsTrue(UE::Flecs::Registration::ResolveScopeTypeName(Object,
			EUnrealFlecsRegistrationScopeType::None).IsNone()));
	}

	TEST_METHOD(ResolveRegistrationScopeToId_ResolvesEveryIdentifierKind)
	{
		const FFlecsEntityHandle ModuleScope = UE::Flecs::Tests::RegistrationScope::CreateModuleScope(
			World(), UE::Flecs::Tests::RegistrationScope::ModuleScopeName);

		World()->RegisterComponentType<FUnrealFlecsPluginTag>();
		const FFlecsEntityHandle PluginScope = World()->CreateEntity(
			UE::Flecs::Tests::RegistrationScope::PluginScopeName.ToString())
			.Add(flecs::Module)
			.Add<FUnrealFlecsPluginTag>();

		const FFlecsEntityHandle CustomNameScope = World()->CreateEntity(
			UE::Flecs::Tests::RegistrationScope::CustomNameScopeName.ToString());
		const FFlecsEntityHandle CustomSymbolScope = World()->CreateEntity(TEXT("RegistrationScopeCustomSymbol"));
		CustomSymbolScope.GetEntity().set_symbol("RegistrationScopeCustomSymbolIdentifier");

		const FFlecsId ModuleScopeId = UE::Flecs::Registration::ResolveRegistrationScopeToId(World(),
			UE::Flecs::Tests::RegistrationScope::ModuleScopeName, EUnrealFlecsRegistrationScopeType::Module);
		const FFlecsId PluginScopeId = UE::Flecs::Registration::ResolveRegistrationScopeToId(World(),
			UE::Flecs::Tests::RegistrationScope::PluginScopeName, EUnrealFlecsRegistrationScopeType::Plugin);
		const FFlecsId CustomNameScopeId = UE::Flecs::Registration::ResolveRegistrationScopeToId(World(),
			UE::Flecs::Tests::RegistrationScope::CustomNameScopeName,
			EUnrealFlecsRegistrationScopeType::CustomNameIdentifier);
		const FFlecsId CustomSymbolScopeId = UE::Flecs::Registration::ResolveRegistrationScopeToId(World(),
			FName(TEXT("RegistrationScopeCustomSymbolIdentifier")),
			EUnrealFlecsRegistrationScopeType::CustomSymbolIdentifier);
		const FFlecsId NoneScopeId = UE::Flecs::Registration::ResolveRegistrationScopeToId(World(),
			UE::Flecs::Tests::RegistrationScope::ModuleScopeName, EUnrealFlecsRegistrationScopeType::None);

		ASSERT_THAT(IsTrue(ModuleScopeId == ModuleScope.GetFlecsId()));
		ASSERT_THAT(IsTrue(PluginScopeId == PluginScope.GetFlecsId()));
		ASSERT_THAT(IsTrue(CustomNameScopeId == CustomNameScope.GetFlecsId()));
		ASSERT_THAT(IsTrue(CustomSymbolScopeId == CustomSymbolScope.GetFlecsId()));
		ASSERT_THAT(IsFalse(NoneScopeId.IsValid()));
	}

	TEST_METHOD(ComponentRegistration_AttachesComponentToResolvedScope)
	{
		const FFlecsEntityHandle ModuleScope = UE::Flecs::Tests::RegistrationScope::CreateModuleScope(
			World(), UE::Flecs::Tests::RegistrationScope::ModuleScopeName);

		const FFlecsComponentPropertiesDefinition ComponentProperties
			= FFlecsComponentPropertiesDefinition::Make<FFlecsRegistrationScopeModuleComponent>();
		ComponentProperties.RegistrationFunction(World(), ComponentProperties);

		const FFlecsEntityHandle RegisteredComponent
			= World()->RegisterComponentType<FFlecsRegistrationScopeModuleComponent>();

		ASSERT_THAT(IsTrue(RegisteredComponent.HasPair(flecs::ChildOf, ModuleScope.GetFlecsId())));
	}

	TEST_METHOD(ExplicitScopeName_ScopesRegistrationAndBeginPlayAndRestoresPreviousScope)
	{
		const FFlecsEntityHandle ExplicitScope = UE::Flecs::Tests::RegistrationScope::CreateModuleScope(
			World(), UE::Flecs::Tests::RegistrationScope::ModuleScopeName);
		
		const FFlecsEntityHandle PreviousScope = World()->CreateEntity(TEXT("RegistrationScopePrevious"));
		World()->SetScope(PreviousScope);

		UFlecsExplicitModuleRegistrationScopeTestObject* Object
			= World()->RegisterFlecsObject<UFlecsExplicitModuleRegistrationScopeTestObject>();
		ASSERT_THAT(IsNotNull(Object));
		ASSERT_THAT(IsTrue(Object->GetRegisteredEntity().HasPair(flecs::ChildOf, ExplicitScope.GetFlecsId())));
		ASSERT_THAT(IsTrue(Object->GetBeginPlayEntity().HasPair(flecs::ChildOf, ExplicitScope.GetFlecsId())));
		ASSERT_THAT(IsTrue(World()->GetScope() == PreviousScope.GetFlecsId()));

		Object->ResetBeginPlayEntity();
		World()->CallBeginPlayForRegisteredObjects();

		ASSERT_THAT(IsTrue(Object->GetBeginPlayEntity().HasPair(flecs::ChildOf, ExplicitScope.GetFlecsId())));
		ASSERT_THAT(IsTrue(World()->GetScope() == PreviousScope.GetFlecsId()));
	}

	TEST_METHOD(MissingScope_RegistersAndBeginsUnscopedAndRestoresPreviousScope)
	{
		const FFlecsEntityHandle PreviousScope = World()->CreateEntity(TEXT("RegistrationScopePrevious"));
		World()->SetScope(PreviousScope);

		UFlecsMissingCustomRegistrationScopeTestObject* Object
			= World()->RegisterFlecsObject<UFlecsMissingCustomRegistrationScopeTestObject>();
		ASSERT_THAT(IsNotNull(Object));
		ASSERT_THAT(IsFalse(Object->GetRegisteredEntity().HasPair(flecs::ChildOf, PreviousScope.GetFlecsId())));
		ASSERT_THAT(IsFalse(Object->GetBeginPlayEntity().HasPair(flecs::ChildOf, PreviousScope.GetFlecsId())));
		ASSERT_THAT(IsTrue(World()->GetScope() == PreviousScope.GetFlecsId()));
	}
}; // FlecsRegistrationScopeTests

#endif // #if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS
