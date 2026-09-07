// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Pipelines/FlecsGameLoopInterface.h"

#include "Pipelines/FlecsGameLoopTag.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsGameLoopInterface)

void IFlecsGameLoopInterface::InitializeGameLoop_Internal(TSolidNotNull<UFlecsWorld*> InWorld)
{
	GameLoopEntity = InWorld->CreateEntity()
		.Add<FFlecsGameLoopTag>();

	InitializeGameLoop(InWorld, GameLoopEntity);
}

bool IFlecsGameLoopInterface::IsMainLoop() const
{
	return false;
}