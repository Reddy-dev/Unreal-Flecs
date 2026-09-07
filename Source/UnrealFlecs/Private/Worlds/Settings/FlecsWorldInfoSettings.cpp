// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Worlds/Settings/FlecsWorldInfoSettings.h"

#include "Logging/StructuredLog.h"

#include "Pipelines/TickFunctions/FlecsTickFunction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsWorldInfoSettings)

FFlecsTickFunctionSettingsInfo::FFlecsTickFunctionSettingsInfo()
{
}

TSharedStruct<FFlecsTickFunction> FFlecsTickFunctionSettingsInfo::CreateTickFunctionInstance(
	const FFlecsTickFunctionSettingsInfo& InTickFunctionSettings)
{
	TSharedStruct<FFlecsTickFunction> TickFunctionInstance;
	TickFunctionInstance.Initialize();
	
	FFlecsTickFunction& TickFunction = TickFunctionInstance.Get<FFlecsTickFunction>();
	TickFunction.bCanEverTick = InTickFunctionSettings.bCanEverTick;
	TickFunction.TickGroup = InTickFunctionSettings.TickGroup;
	TickFunction.EndTickGroup = InTickFunctionSettings.EndTickGroup;
	TickFunction.bStartWithTickEnabled = InTickFunctionSettings.bStartWithTickEnabled;
	TickFunction.bAllowTickOnDedicatedServer = InTickFunctionSettings.bAllowTickOnDedicatedServer;
	TickFunction.bTickEvenWhenPaused = InTickFunctionSettings.bTickEvenWhenPaused;
	TickFunction.TickInterval = InTickFunctionSettings.TickInterval;
	TickFunction.bHighPriority = InTickFunctionSettings.bHighPriority;
	TickFunction.bAllowTickBatching = InTickFunctionSettings.bAllowTickBatching;
	TickFunction.bRunTransactionally = InTickFunctionSettings.bRunTransactionally;

	solid_check(TickFunctionInstance.IsValid());
	
	return TickFunctionInstance;
}

FFlecsWorldSettingsInfo::FFlecsWorldSettingsInfo()
{
}
