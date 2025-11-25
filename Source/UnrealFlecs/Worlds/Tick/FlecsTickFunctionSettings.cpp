// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsTickFunctionSettings.h"

#include "FlecsWorldTickFunction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsTickFunctionSettings)

void FFlecsTickFunctionSettings::ApplySettingsToTickFunction(const TSolidNotNull<UFlecsWorld*> InFlecsWorld,
	FFlecsWorldTickFunction& OutTickFunction) const
{
	OutTickFunction.FlecsWorld = InFlecsWorld;
	OutTickFunction.TickGroupTag = TickGroupTag;

	OutTickFunction.TickGroup = TickGroup;
	OutTickFunction.bStartWithTickEnabled = bStartWithTickEnabled;
	OutTickFunction.TickInterval = TickInterval;
	OutTickFunction.bTickEvenWhenPaused = bTickEvenWhenPaused;
	OutTickFunction.bAllowTickBatching = bAllowTickBatching;
	OutTickFunction.bAllowTickOnDedicatedServer = bAllowTickOnDedicatedServer;
	OutTickFunction.EndTickGroup = EndTickGroup;
	OutTickFunction.bHighPriority = bHighPriority;
}
