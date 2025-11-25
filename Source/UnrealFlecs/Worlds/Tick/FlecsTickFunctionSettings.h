// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/EngineBaseTypes.h"
#include "GameplayTagContainer.h"

#include "Types/SolidNotNull.h"

#include "FlecsTickFunctionSettings.generated.h"

class UFlecsWorld;

struct FFlecsWorldTickFunction;

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsTickFunctionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag TickGroupTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ETickingGroup> TickGroup = TG_PrePhysics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStartWithTickEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double TickInterval = 0.0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTickEvenWhenPaused = false;

	// @TODO: expose and test
	UPROPERTY()
	bool bAllowTickBatching = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	bool bAllowTickOnDedicatedServer = true;
	
	// @TODO: currently unused
	UPROPERTY()
	TEnumAsByte<ETickingGroup> EndTickGroup = TG_LastDemotable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	bool bHighPriority = false;

	void ApplySettingsToTickFunction(const TSolidNotNull<UFlecsWorld*> InFlecsWorld,
		FFlecsWorldTickFunction& OutTickFunction) const;
	
}; // struct FFlecsTickFunctionSettings