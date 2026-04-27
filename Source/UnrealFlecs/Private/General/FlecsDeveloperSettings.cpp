// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "General/FlecsDeveloperSettings.h"

#include "flecs.h"

#include "HAL/IConsoleManager.h"

#include "General/FlecsModuleSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsDeveloperSettings)

static TAutoConsoleVariable<bool> CVarEnableFlecs(
	TEXT("Flecs.UseFlecs"),
	true,
	TEXT("Enable Unreal Flecs Plugin.")
);

// Sets the Flecs log level based on the console variable and when the CVar changes
static void FlecsLogLevelSink()
{
	if LIKELY_IF(const IConsoleVariable* const CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("Flecs.LogLevel")))
	{
		const int32 NewLevel = CVar->GetInt();
		ecs_log_set_level(NewLevel);
	}
}

static TAutoConsoleVariable<int32> CVarFlecsLogLevel(
	TEXT("Flecs.LogLevel"),
	-1,
	TEXT("Sets the log level for Flecs. \n"),
	ECVF_Default);

static FAutoConsoleVariableSink CVarFlecsLogLevelSink(FConsoleCommandDelegate::CreateStatic(&FlecsLogLevelSink));

UFlecsDeveloperSettings::UFlecsDeveloperSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlecsDeveloperSettings::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	
	for (TFieldIterator<FProperty> It(GetClass()); It; ++It)
	{
		FProperty* Property = *It;
		solid_checkf(Property, TEXT("Failed to get property from iterator for class %s"), *GetClass()->GetName());

		if (Property->HasAnyPropertyFlags(CPF_Config) && Property->HasMetaData(TEXT("ConsoleVariable")))
		{
			ExportValuesToConsoleVariables(Property);
		}
	}

#endif // WITH_EDITOR
}

#if WITH_EDITOR

void UFlecsDeveloperSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->HasAnyPropertyFlags(CPF_Config) && PropertyChangedEvent.Property->HasMetaData(TEXT("ConsoleVariable")))
	{
		ExportValuesToConsoleVariables(PropertyChangedEvent.Property);
	}
}

#endif // WITH_EDITOR
