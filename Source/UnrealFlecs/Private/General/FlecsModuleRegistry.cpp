// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "General/FlecsModuleRegistry.h"

#include "SolidMacros/Macros.h"

#include "Worlds/FlecsWorld.h"

UE::Flecs::FFlecsModuleRegistry& UE::Flecs::FFlecsModuleRegistry::Get()
{
	static FFlecsModuleRegistry Singleton;
	return Singleton;
}

void UE::Flecs::FFlecsModuleRegistry::RegisterUnrealFlecsModule(const FName& ModuleName)
{
	solid_checkf(!ModuleName.IsNone(), TEXT("ModuleName cannot be None when registering an Unreal Flecs module"));
	solid_checkf(!RegisteredModules.Contains(ModuleName), 
		TEXT("Module %s is already registered in the Unreal Flecs module registry"), *ModuleName.ToString());
	
	RegisteredModules.Add(ModuleName);
}

void UE::Flecs::FFlecsModuleRegistry::InitializeRegisteredModules(
	const TSolidNotNull<const UFlecsWorld*> InFlecsWorld) const
{
	for (const FName& ModuleName : RegisteredModules)
	{
		InFlecsWorld->CreateEntity(ModuleName.ToString())
			.Add(flecs::Module);
	}
}
