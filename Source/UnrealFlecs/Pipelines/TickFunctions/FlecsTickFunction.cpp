// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsTickFunction.h"

#include "Logs/FlecsCategories.h"

#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsTickFunction)

void FFlecsTickFunction::ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread,
	const FGraphEventRef& MyCompletionGraphEvent)
{
	if UNLIKELY_IF(!OwningWorld)
	{
		UE_LOGFMT(LogFlecsWorld, Error,
			"Flecs Tick Function executed but OwningWorld is null for Tick Type Tag: {TickTypeTag}",
			TickTypeTag.ToString());
		return;
	}
	
	OwningWorld->ProgressGameLoops(TickTypeTag, DeltaTime);
}

FString FFlecsTickFunction::DiagnosticMessage()
{
	return FString::Printf(TEXT("Flecs Tick Function for Tick Type Tag: %s"), *TickTypeTag.ToString());
}
