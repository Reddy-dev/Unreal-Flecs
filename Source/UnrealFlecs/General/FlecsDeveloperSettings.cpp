// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsDeveloperSettings.h"

#include "HAL/IConsoleManager.h"

#include "flecs.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsDeveloperSettings)

static bool bEnableFlecs = true;
static FAutoConsoleVariableRef CVarEnableFlecs(
	TEXT("Flecs.UseFlecs"),
	bEnableFlecs,
	TEXT("Enable Unreal Flecs Plugin.")
);

static bool bUseTaskThreads = false;
static FAutoConsoleVariableRef CVarUseTaskThreads(
	TEXT("Flecs.UseTaskThreads"),
	bUseTaskThreads,
	TEXT("Enable task threads for Flecs.")
);

static int32 TaskThreadCount = 4;
static FAutoConsoleVariableRef CVarTaskThreadCount(
	TEXT("Flecs.TaskThreadCount"),
	TaskThreadCount,
	TEXT("Number of threads to use for Flecs task processing.")
);

static int32 FlecsLogLevel = -1;

static void FlecsLogLevelSink()
{
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("Flecs.LogLevel")))
	{
		const int32 NewLevel = CVar->GetInt();
		FlecsLogLevel = NewLevel;
		ecs_log_set_level(NewLevel);
	}
}

static FAutoConsoleVariableRef CVarFlecsLogLevel(
	TEXT("Flecs.LogLevel"),
	FlecsLogLevel,
	TEXT("Sets the log level for Flecs. \n"),
	ECVF_Default);

static FAutoConsoleVariableSink CVarFlecsLogLevelSink(FConsoleCommandDelegate::CreateStatic(&FlecsLogLevelSink));

void UFlecsDeveloperSettings::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	
	if (IsTemplate())
	{
		//ImportConsoleVariableValues();
	}

#endif // WITH_EDITOR
}

#if WITH_EDITOR

void UFlecsDeveloperSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		ExportValuesToConsoleVariables(PropertyChangedEvent.Property);
	}
}

#endif // WITH_EDITOR
