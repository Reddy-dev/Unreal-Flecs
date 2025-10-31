// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsGameFrameworkModule.h"

#include "Components/FlecsTransformTypes.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsGameFrameworkModule)

void UFlecsGameFrameworkModule::InitializeModule(
	TSolidNotNull<UFlecsWorld*> InWorld, const FFlecsEntityHandle& InModuleEntity)
{
	InWorld->RegisterComponentType<FFlecsVectorComponent>();
	InWorld->RegisterComponentType<FFlecsQuatComponent>();
}

void UFlecsGameFrameworkModule::DeinitializeModule(TSolidNotNull<UFlecsWorld*> InWorld)
{
	
}
