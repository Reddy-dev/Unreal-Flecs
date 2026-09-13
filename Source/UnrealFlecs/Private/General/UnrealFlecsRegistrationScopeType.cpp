// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "General/UnrealFlecsRegistrationScopeType.h"

#include "Interfaces/IPluginManager.h"

#include "General/FlecsModuleRegistry.h"
#include "Logs/FlecsCategories.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UnrealFlecsRegistrationScopeType)

TTuple<FString, EUnrealFlecsRegistrationScopeType> UE::Flecs::Registration::ResolveScopeTypeName(
	const TSolidNotNull<const UObject*> InObject, 
	const EUnrealFlecsRegistrationScopeType InScopeType)
{
	const FString ModuleName = FPackageName::GetShortName(InObject->GetClass()->GetOuterUPackage()->GetName());

	switch (InScopeType)
	{
		case EUnrealFlecsRegistrationScopeType::Module:
			{
				return MakeTuple(ModuleName, InScopeType);
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
					UE_LOGFMT(LogFlecsCore, Warning,
						"Could not infer a plugin scope for registered object {ObjectName}: native module {ModuleName} has no owning plugin. Set an explicit scope name or use another scope type.",
						InObject->GetFName(), ModuleName);
					return MakeTuple("", InScopeType);
				}
				
				return MakeTuple((*OwningPlugin)->GetName(), InScopeType);
			}

		case EUnrealFlecsRegistrationScopeType::CustomNameIdentifier:
		case EUnrealFlecsRegistrationScopeType::CustomSymbolIdentifier:
			UE_LOGFMT(LogFlecsCore, Warning,
				"Registered object {ObjectName} uses scope type {ScopeType}, which requires an explicit scope name.",
				InObject->GetFName(), StaticEnum<EUnrealFlecsRegistrationScopeType>()->GetNameStringByValue(static_cast<int64>(InScopeType)));
				
			return MakeTuple("", InScopeType);

		case EUnrealFlecsRegistrationScopeType::None:
			return MakeTuple("", InScopeType);
		case EUnrealFlecsRegistrationScopeType::Unset:
			if LIKELY_IF(const FFlecsModuleRegistryRegisteredItem* Item 
				= FFlecsModuleRegistry::Get().FindRegisteredModule(FName(ModuleName)))
			{
				return ResolveScopeTypeName(InObject, Item->DefaultScopeType);
			}
			
			return MakeTuple("", InScopeType);
		default: ;
	}
	
	
	// @TODO: error?
	return MakeTuple("", EUnrealFlecsRegistrationScopeType::Unset);
}

FFlecsId UE::Flecs::Registration::ResolveRegistrationScopeToId(
	const TSolidNotNull<const UFlecsWorld*> InFlecsWorld,
	const FString& ScopeName, 
	const EUnrealFlecsRegistrationScopeType InScopeType)
{
	if UNLIKELY_IF(InScopeType == EUnrealFlecsRegistrationScopeType::None)
	{
		return FFlecsId::Null();
	}
	
	if UNLIKELY_IF(ScopeName.IsEmpty())
	{
		return FFlecsId::Null();
	}

	switch (InScopeType)
	{
		case EUnrealFlecsRegistrationScopeType::Module:
			return InFlecsWorld->GetFlecsModule(FName(*ScopeName));
		case EUnrealFlecsRegistrationScopeType::Plugin:
			return InFlecsWorld->GetFlecsPlugin(FName(*ScopeName));
		case EUnrealFlecsRegistrationScopeType::CustomNameIdentifier:
			return InFlecsWorld->LookupEntity(ScopeName);
		case EUnrealFlecsRegistrationScopeType::CustomSymbolIdentifier:
			return InFlecsWorld->LookupEntityBySymbol_Internal(ScopeName);
		case EUnrealFlecsRegistrationScopeType::Unset:
		case EUnrealFlecsRegistrationScopeType::None: 
		default:
			return FFlecsId::Null();
	}
}
