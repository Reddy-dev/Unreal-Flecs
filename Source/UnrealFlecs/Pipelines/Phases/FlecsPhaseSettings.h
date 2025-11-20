// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

#include "FlecsPhaseSettings.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsPhaseSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Flecs|Pipeline Phase")
	FGameplayTag PhaseTag;
	
}; // struct FFlecsPhaseSettings
