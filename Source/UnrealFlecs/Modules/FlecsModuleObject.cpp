// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsModuleObject.h"

#include "Logs/FlecsCategories.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsModuleObject)

UFlecsModuleObject::UFlecsModuleObject()
{
}

UFlecsModuleObject::UFlecsModuleObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UFlecsModuleObject::HasHardDependency(const TSubclassOf<UFlecsModuleInterface> ModuleClass) const
{
	return HardModuleDependencies.Contains(ModuleClass);
}

void UFlecsModuleObject::AddHardDependency(const TSubclassOf<UFlecsModuleInterface> ModuleClass)
{
	if UNLIKELY_IF(ensureAlwaysMsgf(!IsValid(ModuleClass), TEXT("ModuleClass is not valid!")))
	{
		return;
	}

	HardModuleDependencies.AddUnique(ModuleClass);
}

void UFlecsModuleObject::RegisterSoftDependency(const TSubclassOf<UFlecsModuleInterface> ModuleClass,
	const FFlecsDependencyFunctionDefinition::FDependencyFunctionType& DependencyFunction)
{
	if UNLIKELY_IF(ensureAlwaysMsgf(!IsValid(ModuleClass), TEXT("ModuleClass is not valid!")))
	{
		return;
	}
	
	GetFlecsWorld()->RegisterModuleDependency(this, ModuleClass, DependencyFunction);
}

TArray<TSubclassOf<UFlecsModuleInterface>> UFlecsModuleObject::GetHardDependentModuleClasses() const
{
	return HardModuleDependencies;
}
