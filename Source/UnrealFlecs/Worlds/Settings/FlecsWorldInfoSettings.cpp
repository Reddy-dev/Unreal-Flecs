// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsWorldInfoSettings.h"

#include "Logging/StructuredLog.h"

#include "Logs/FlecsCategories.h"

#include "Pipelines/TickFunctions/FlecsTickFunction.h"
#include "Pipelines/TickFunctions/FlecsTickTypeNativeTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsWorldInfoSettings)

FFlecsTickFunctionSettingsInfo::FFlecsTickFunctionSettingsInfo()
{
}

FFlecsTickFunctionSettingsInfo FFlecsTickFunctionSettingsInfo::GetTickFunctionSettingsDefault(
	const FGameplayTag& InTickTypeTag)
{
	FFlecsTickFunctionSettingsInfo TickFunctionSettings = FFlecsTickFunctionSettingsInfo();
	TickFunctionSettings.bStartWithTickEnabled = true;
	TickFunctionSettings.bAllowTickOnDedicatedServer = true;
	TickFunctionSettings.bTickEvenWhenPaused = false;
	TickFunctionSettings.TickInterval = 0.0f;
	TickFunctionSettings.TickTypeTag = InTickTypeTag;

	if (InTickTypeTag == FlecsTickType_MainLoop)
	{
		TickFunctionSettings.TickFunctionName = TEXT("MainLoopTickFunction");
		TickFunctionSettings.TickGroup = ETickingGroup::TG_PrePhysics;
		TickFunctionSettings.EndTickGroup = ETickingGroup::TG_PrePhysics;
		TickFunctionSettings.bTickEvenWhenPaused = true;
	}
	else if (InTickTypeTag == FlecsTickType_PrePhysics)
	{
		TickFunctionSettings.TickFunctionName = TEXT("PrePhysicsTickFunction");
		TickFunctionSettings.TickGroup = ETickingGroup::TG_PrePhysics;
		TickFunctionSettings.EndTickGroup = ETickingGroup::TG_PrePhysics;
		
		TickFunctionSettings.TickFunctionPrerequisiteTags.Add(FlecsTickType_MainLoop);
	}
	else if (InTickTypeTag == FlecsTickType_DuringPhysics)
	{
		TickFunctionSettings.TickFunctionName = TEXT("DuringPhysicsTickFunction");
		TickFunctionSettings.TickGroup = ETickingGroup::TG_DuringPhysics;
		TickFunctionSettings.EndTickGroup = ETickingGroup::TG_DuringPhysics;
	}
	else if (InTickTypeTag == FlecsTickType_PostPhysics)
	{
		TickFunctionSettings.TickFunctionName = TEXT("PostPhysicsTickFunction");
		TickFunctionSettings.TickGroup = ETickingGroup::TG_PostPhysics;
		TickFunctionSettings.EndTickGroup = ETickingGroup::TG_PostPhysics;
	}
	else if (InTickTypeTag == FlecsTickType_PostUpdateWork)
	{
		TickFunctionSettings.TickFunctionName = TEXT("PostUpdateWorkTickFunction");
		TickFunctionSettings.TickGroup = ETickingGroup::TG_PostUpdateWork;
		TickFunctionSettings.EndTickGroup = ETickingGroup::TG_LastDemotable;
	}
	else
	{
		UE_LOGFMT(LogFlecsWorld, Warning,
			"Unknown TickTypeTag {TickTypeTag}, defaulting to PrePhysics settings.",
			*InTickTypeTag.ToString());
		
		TickFunctionSettings.TickFunctionName = TEXT("PrePhysicsTickFunction");
		TickFunctionSettings.TickGroup = ETickingGroup::TG_PrePhysics;
		TickFunctionSettings.EndTickGroup = ETickingGroup::TG_PrePhysics;
	}
	
	return TickFunctionSettings;
}

TSharedStruct<FFlecsTickFunction> FFlecsTickFunctionSettingsInfo::CreateTickFunctionInstance(
	const FFlecsTickFunctionSettingsInfo& InTickFunctionSettings)
{
	TSharedStruct<FFlecsTickFunction> TickFunctionInstance;
	TickFunctionInstance.Initialize();
	
	FFlecsTickFunction& TickFunction = TickFunctionInstance.Get<FFlecsTickFunction>();
	TickFunction.TickTypeTag = InTickFunctionSettings.TickTypeTag;
	TickFunction.TickGroup = InTickFunctionSettings.TickGroup;
	TickFunction.EndTickGroup = InTickFunctionSettings.EndTickGroup;
	TickFunction.bStartWithTickEnabled = InTickFunctionSettings.bStartWithTickEnabled;
	TickFunction.bAllowTickOnDedicatedServer = InTickFunctionSettings.bAllowTickOnDedicatedServer;
	TickFunction.bTickEvenWhenPaused = InTickFunctionSettings.bTickEvenWhenPaused;
	TickFunction.TickInterval = InTickFunctionSettings.TickInterval;
	TickFunction.bHighPriority = InTickFunctionSettings.bHighPriority;
	TickFunction.bAllowTickBatching = InTickFunctionSettings.bAllowTickBatching;
	TickFunction.bRunTransactionally = InTickFunctionSettings.bRunTransactionally;
	
	TickFunction.bCanEverTick = true;

	solid_check(TickFunctionInstance.IsValid());
	
	return TickFunctionInstance;
}

FFlecsWorldSettingsInfo::FFlecsWorldSettingsInfo()
{
	FFlecsTickFunctionSettingsInfo MainLoopTickFunctionSettings = FFlecsTickFunctionSettingsInfo::GetTickFunctionSettingsDefault(
		FlecsTickType_MainLoop);
	
	FFlecsTickFunctionSettingsInfo PrePhysicsTickFunctionSettings = FFlecsTickFunctionSettingsInfo::GetTickFunctionSettingsDefault(
		FlecsTickType_PrePhysics);

	FFlecsTickFunctionSettingsInfo DuringPhysicsTickFunctionSettings = FFlecsTickFunctionSettingsInfo::GetTickFunctionSettingsDefault(
		FlecsTickType_DuringPhysics);

	FFlecsTickFunctionSettingsInfo PostPhysicsTickFunctionSettings = FFlecsTickFunctionSettingsInfo::GetTickFunctionSettingsDefault(
		FlecsTickType_PostPhysics);

	FFlecsTickFunctionSettingsInfo PostUpdateWorkTickFunctionSettings = FFlecsTickFunctionSettingsInfo::GetTickFunctionSettingsDefault(
		FlecsTickType_PostUpdateWork);

	TickFunctions.Add(MainLoopTickFunctionSettings);
	TickFunctions.Add(PrePhysicsTickFunctionSettings);
	TickFunctions.Add(DuringPhysicsTickFunctionSettings);
	TickFunctions.Add(PostPhysicsTickFunctionSettings);
	TickFunctions.Add(PostUpdateWorkTickFunctionSettings);
}
