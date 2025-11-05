// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsDefaultGameLoop.h"

#include "UnrealFlecsPhaseTag.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsDefaultGameLoop)

UFlecsDefaultGameLoop::UFlecsDefaultGameLoop()
{
}

void UFlecsDefaultGameLoop::InitializeGameLoop(TSolidNotNull<UFlecsWorld*> InWorld)
{
	GameLoopEntity = InWorld->CreateEntity("DefaultGameLoop")
		.Add(flecs::Module);

	GameLoopEntity.Scope([this, InWorld]()
	{
		PhaseTree.CreatePhasesFromTree(InWorld);
	});
}

bool UFlecsDefaultGameLoop::Progress(const double DeltaTime, const TSolidNotNull<UFlecsWorld*> InWorld)
{
	InWorld->Progress(DeltaTime);
	return true;
}

#if WITH_EDITOR

EDataValidationResult UFlecsDefaultGameLoop::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = EDataValidationResult::Valid;
	
	const EDataValidationResult SuperResult = Super::IsDataValid(Context);

	const EDataValidationResult PhaseTreeResult = PhaseTree.IsDataValid(Context);

	Result = CombineDataValidationResults(Result, SuperResult);
	Result = CombineDataValidationResults(Result, PhaseTreeResult);
	return Result;
}

#endif // WITH_EDITOR
