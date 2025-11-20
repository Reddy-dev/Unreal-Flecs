// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/EngineBaseTypes.h"

#include "GameplayTagContainer.h"

#include "FlecsPhaseTickFunction.generated.h"

USTRUCT()
struct UNREALFLECS_API FFlecsPhaseTickFunction : public FTickFunction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Flecs|World")
	FGameplayTag PhaseTag;
	
}; // struct FFlecsPhaseTickFunction




