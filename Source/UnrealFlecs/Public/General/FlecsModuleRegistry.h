// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "Containers/Set.h"
#include "UObject/NameTypes.h"

#include "UnrealFlecsRegistrationScopeType.h"

#include "Types/SolidNotNull.h"

class UFlecsWorld;

struct UNREALFLECS_API FFlecsModuleRegistryRegisteredItem
{
	NO_DISCARD friend uint32 GetTypeHash(const FFlecsModuleRegistryRegisteredItem& InItem)
	{
		return GetTypeHash(InItem.Name);
	}
	
	NO_DISCARD friend bool operator==(const FFlecsModuleRegistryRegisteredItem&, const FFlecsModuleRegistryRegisteredItem&) = default;
	
	FName Name;
	EUnrealFlecsRegistrationScopeType DefaultScopeType = EUnrealFlecsRegistrationScopeType::Module;
}; // struct FFlecsModuleRegistryRegisteredItem

namespace UE::Flecs
{
	struct UNREALFLECS_API FFlecsModuleRegistry
	{
		static FFlecsModuleRegistry& Get();

	public:
		void RegisterUnrealFlecsModule(const FName& ModuleName, 
			const EUnrealFlecsRegistrationScopeType InScopeType = EUnrealFlecsRegistrationScopeType::Module);
		void RegisterUnrealFlecsPlugin(const FName& InPluginName, 
			const EUnrealFlecsRegistrationScopeType InScopeType = EUnrealFlecsRegistrationScopeType::Plugin);
		
		void InitializeRegisteredModules(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld) const;
		
		NO_DISCARD const FFlecsModuleRegistryRegisteredItem* FindRegisteredModule(const FName& InModuleName) const;
		NO_DISCARD const FFlecsModuleRegistryRegisteredItem* FindRegisteredPlugin(const FName& InPluginName) const;
		
	private:
		TSet<FFlecsModuleRegistryRegisteredItem> RegisteredModules;
		TSet<FFlecsModuleRegistryRegisteredItem> RegisteredPlugins;
		
	}; // struct FFlecsModuleRegistry
	
} // namespace UE::Flecs

