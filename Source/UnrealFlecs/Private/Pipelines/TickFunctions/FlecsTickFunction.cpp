// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Pipelines/TickFunctions/FlecsTickFunction.h"

#include "Logs/FlecsCategories.h"
#include "Pipelines/FlecsGameLoopInterface.h"

#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsTickFunction)

void FFlecsTickFunction::ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread,
	const FGraphEventRef& MyCompletionGraphEvent)
{
	if UNLIKELY_IF(!IsValid(OwningGameLoop))
	{
		UE_LOGFMT(LogFlecsCore, Error, 
			"Flecs Tick Function for Tick Type Tag: has no valid OwningGameLoop!");
		return;
	}
	
	const TSolidNotNull<IFlecsGameLoopInterface*> GameLoop = CastChecked<IFlecsGameLoopInterface>(OwningGameLoop);
	GameLoop->Progress(DeltaTime, OwningWorld, 
		TickType, CurrentThread, MyCompletionGraphEvent);
}

FString FFlecsTickFunction::DiagnosticMessage()
{
	return FString::Format(TEXT("Flecs Tick Function for Tick Type Tag: {0}"), { *OwningGameLoop->GetName() } );
}
