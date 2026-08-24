// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "General/UnrealFlecsRegistrationScopeType.h"

#include "Interfaces/IPluginManager.h"

#include "Logs/FlecsCategories.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UnrealFlecsRegistrationScopeType)

FName UE::Flecs::Registration::ResolveScopeTypeName(
	const TSolidNotNull<const UObject*> InObject, 
	const EUnrealFlecsRegistrationScopeType InScopeType)
{
	const FName ModuleName(*FPackageName::GetShortName(InObject->GetClass()->GetOuterUPackage()->GetName()));

	switch (InScopeType)
	{
		case EUnrealFlecsRegistrationScopeType::Module:
			{
				return ModuleName;
				break;
			}
		case EUnrealFlecsRegistrationScopeType::Plugin:
			{
				const TArray<TSharedRef<IPlugin>> DiscoveredPlugins = IPluginManager::Get().GetDiscoveredPlugins();
				
				const TSharedRef<IPlugin>* OwningPlugin = DiscoveredPlugins.FindByPredicate(
					[ModuleName](const TSharedRef<IPlugin>& Plugin)
					{
						return Plugin->GetDescriptor().Modules.ContainsByPredicate(
							[ModuleName](const FModuleDescriptor& Module)
							{
								return Module.Name == ModuleName;
							});
					});

				if UNLIKELY_IF(!OwningPlugin)
				{
					UE_LOGFMT(LogFlecsCore, Error,
						"Could not infer a plugin scope for registered object {ObjectName}: native module {ModuleName} has no owning plugin. Set an explicit scope name or use another scope type.",
						InObject->GetFName(), ModuleName);
					return NAME_None;
				}

				return FName((*OwningPlugin)->GetName());
			}

		case EUnrealFlecsRegistrationScopeType::CustomNameIdentifier:
		case EUnrealFlecsRegistrationScopeType::CustomSymbolIdentifier:
			UE_LOGFMT(LogFlecsCore, Error,
				"Registered object {ObjectName} uses scope type {ScopeType}, which requires an explicit scope name.",
				InObject->GetFName(), StaticEnum<EUnrealFlecsRegistrationScopeType>()->GetNameStringByValue(static_cast<int64>(InScopeType)));
				
			return NAME_None;

		case EUnrealFlecsRegistrationScopeType::None: UNLIKELY_ATTRIBUTE
			solid_cassume(false);
			return NAME_None;
	}
	
	
	// @TODO: error?
	return NAME_None;
}

FFlecsId UE::Flecs::Registration::ResolveRegistrationScopeToId(
	const TSolidNotNull<const UFlecsWorld*> InFlecsWorld,
	const FName& ScopeName, 
	const EUnrealFlecsRegistrationScopeType InScopeType)
{
	if (InScopeType == EUnrealFlecsRegistrationScopeType::None)
	{
		return FFlecsId::Null();
	}

	switch (InScopeType)
	{
		case EUnrealFlecsRegistrationScopeType::Module:
			return InFlecsWorld->GetFlecsModule(ScopeName);
		case EUnrealFlecsRegistrationScopeType::Plugin:
			return InFlecsWorld->GetFlecsPlugin(ScopeName);
		case EUnrealFlecsRegistrationScopeType::CustomNameIdentifier:
			return InFlecsWorld->LookupEntity(ScopeName.ToString());
		case EUnrealFlecsRegistrationScopeType::CustomSymbolIdentifier:
			return InFlecsWorld->LookupEntityBySymbol_Internal(ScopeName.ToString());
		case EUnrealFlecsRegistrationScopeType::None: 
		default:
			return FFlecsId::Null();
	}
}
