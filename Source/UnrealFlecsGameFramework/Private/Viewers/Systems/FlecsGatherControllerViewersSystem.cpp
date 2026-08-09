// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Viewers/Systems/FlecsGatherControllerViewersSystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsGatherControllerViewersSystem)


UFlecsGatherControllerViewersSystem::UFlecsGatherControllerViewersSystem(const FObjectInitializer& ObjectInitializer)
{
}

void UFlecsGatherControllerViewersSystem::BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld,
	TFlecsSystemBuilder<>& InBuilder) const
{
	Super::BuildSystem(InWorld, InBuilder);
}

void UFlecsGatherControllerViewersSystem::RunEachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
                                                          flecs::iter& InIterator)
{
	
}
