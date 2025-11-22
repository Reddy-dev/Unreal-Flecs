// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsWorldTickFunction.h"

#include "Logging/StructuredLog.h"

#include "SolidMacros/Macros.h"

#include "Logs/FlecsCategories.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FFlecsWorldTickFunction)

void FFlecsWorldTickFunction::ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread,
	const FGraphEventRef& MyCompletionGraphEvent)
{
	if LIKELY_IF(FlecsWorld.IsValid())
	{
		FlecsWorld->ProgressGameLoops(TickGroup, DeltaTime);
	}
	else
	{
		UE_LOGFMT(LogFlecsWorld, Error, "FlecsWorldSubsystem is no longer valid during Flecs Phase Tick");
	}
}
