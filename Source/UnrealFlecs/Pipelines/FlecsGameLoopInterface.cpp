// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsGameLoopInterface.h"

#include "FlecsGameLoopTag.h"
#include "FlecsTickGroupNativeTags.h"
#include "Ticker/FlecsFixedTickSystemTag.h"
#include "Phases/FlecsDefaultPhaseNativeTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsGameLoopInterface)

// Add default functionality here for any IFlecsGameLoopInterface functions that are not pure virtual.

void IFlecsGameLoopInterface::InitializeModule(const TSolidNotNull<UFlecsWorld*> InWorld,
	const FFlecsEntityHandle& InModuleEntity)
{
	IFlecsModuleInterface::InitializeModule(InWorld, InModuleEntity);

	InModuleEntity.Add<FFlecsGameLoopTag>();

	InitializeGameLoop(InWorld, InModuleEntity);
}

bool IFlecsGameLoopInterface::IsMainLoop() const
{
	return false;
}

FGameplayTag IFlecsGameLoopInterface::GetTickFunctionTag() const
{
	return FFlecsTickGroupNativeTags::Get().FlecsTickGroup_PrePhysics;
}