// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "General/FlecsModuleRegistry.h"

#include "SolidMacros/Macros.h"

#include "Components/UnrealFlecsModuleTag.h"
#include "Components/UnrealFlecsPluginTag.h"
#include "Worlds/FlecsWorld.h"

UE::Flecs::FFlecsModuleRegistry& UE::Flecs::FFlecsModuleRegistry::Get()
{
	static FFlecsModuleRegistry Singleton;
	return Singleton;
}

void UE::Flecs::FFlecsModuleRegistry::RegisterUnrealFlecsModule(const FName& ModuleName,
	const EUnrealFlecsRegistrationScopeType InScopeType)
{
	solid_checkf(!ModuleName.IsNone(), TEXT("ModuleName cannot be None when registering an Unreal Flecs module"));
	
	FFlecsModuleRegistryRegisteredItem RegisteredItem{.Name = ModuleName, .DefaultScopeType = InScopeType};
	
	solid_checkf(!RegisteredModules.Contains(RegisteredItem), 
		TEXT("Module %s is already registered in the Unreal Flecs module registry"), *ModuleName.ToString());
	
	RegisteredModules.Add(MoveTemp(RegisteredItem));
}

void UE::Flecs::FFlecsModuleRegistry::RegisterUnrealFlecsPlugin(const FName& InPluginName,
	const EUnrealFlecsRegistrationScopeType InScopeType)
{
	solid_checkf(!InPluginName.IsNone(), TEXT("PluginName cannot be None when registering an Unreal Flecs plugin"));
	
	FFlecsModuleRegistryRegisteredItem RegisteredItem{.Name = InPluginName, .DefaultScopeType = InScopeType};
	
	solid_checkf(!RegisteredPlugins.Contains(RegisteredItem), 
		TEXT("Module %s is already registered in the Unreal Flecs module registry"), *InPluginName.ToString());
	
	RegisteredPlugins.Add(MoveTemp(RegisteredItem));
}

void UE::Flecs::FFlecsModuleRegistry::InitializeRegisteredModules(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld) const
{
	InFlecsWorld->RegisterComponentType<FUnrealFlecsModuleTag>(false, true)
		.AddPair(flecs::With, flecs::Module);
	
	InFlecsWorld->RegisterComponentType<FUnrealFlecsPluginTag>(false, true)
		.AddPair(flecs::With, flecs::Module);
	
	for (const auto& [Name, DefaultScopeType] : RegisteredPlugins)
	{
		InFlecsWorld->CreateEntity(Name.ToString())
			.Add<FUnrealFlecsPluginTag>();
	}
	
	for (const auto& [Name, DefaultScopeType] : RegisteredModules)
	{
		InFlecsWorld->CreateEntity(Name.ToString())
			.Add<FUnrealFlecsModuleTag>();
	}
}

const FFlecsModuleRegistryRegisteredItem* UE::Flecs::FFlecsModuleRegistry::FindRegisteredModule(
	const FName& InModuleName) const
{
	return RegisteredModules.Find(FFlecsModuleRegistryRegisteredItem{.Name = InModuleName});
}

const FFlecsModuleRegistryRegisteredItem* UE::Flecs::FFlecsModuleRegistry::FindRegisteredPlugin(
	const FName& InPluginName) const
{
	return RegisteredPlugins.Find(FFlecsModuleRegistryRegisteredItem{.Name = InPluginName});
}
