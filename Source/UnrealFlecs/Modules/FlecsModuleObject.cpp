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

bool UFlecsModuleObject::HasHardDependency(const TSubclassOf<UObject> ModuleClass, const bool bAllowChildClasses) const
{
	if UNLIKELY_IF(!ensureAlwaysMsgf(IsValid(ModuleClass), TEXT("ModuleClass is not valid!")))
	{
		return false;
	}

	for (const FFlecsModuleHardDependency& HardDependency : HardModuleDependencies)
	{
		if (bAllowChildClasses)
		{
			if (ModuleClass->IsChildOf(HardDependency.ModuleClass))
			{
				return true;
			}
		}
		else if (HardDependency.ModuleClass == ModuleClass)
		{
			return true;
		}
	}

	return false;
}

void UFlecsModuleObject::AddHardDependency(const TSubclassOf<UObject> ModuleClass, const bool bAllowChildClasses)
{
	if UNLIKELY_IF(!ensureAlwaysMsgf(IsValid(ModuleClass), TEXT("ModuleClass is not valid!")))
	{
		return;
	}

	if UNLIKELY_IF(!ensureAlwaysMsgf(ModuleClass->ImplementsInterface(UFlecsModuleInterface::StaticClass()),
		TEXT("ModuleClass %s does not implement IFlecsModuleInterface!"), *ModuleClass->GetName()))
	{
		return;
	}

	HardModuleDependencies.AddUnique(FFlecsModuleHardDependency{
		.ModuleClass = ModuleClass,
		.bAllowChildClasses = bAllowChildClasses
	});
}

void UFlecsModuleObject::RegisterSoftDependency(const TSubclassOf<UObject> ModuleClass,
	const FFlecsDependencyFunctionDefinition::FDependencyFunctionType& DependencyFunction)
{
	if UNLIKELY_IF(!ensureAlwaysMsgf(IsValid(ModuleClass), TEXT("ModuleClass is not valid!")))
	{
		return;
	}
	
	GetFlecsWorld()->RegisterModuleDependency(this, ModuleClass, DependencyFunction);
}

TArray<FFlecsModuleHardDependency> UFlecsModuleObject::GetHardDependentModuleClasses() const
{
	return HardModuleDependencies;
}
