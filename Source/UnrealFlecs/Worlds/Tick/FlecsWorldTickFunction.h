// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/EngineBaseTypes.h"

#include "GameplayTagContainer.h"

#include "FlecsWorldTickFunction.generated.h"

class UFlecsWorld;

USTRUCT()
struct UNREALFLECS_API FFlecsWorldTickFunction : public FTickFunction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Flecs|World")
	FGameplayTag TickGroupTag;

	UPROPERTY()
	TWeakObjectPtr<UFlecsWorld> FlecsWorld;

	virtual void ExecuteTick(
		float DeltaTime,
		ELevelTick TickType,
		ENamedThreads::Type CurrentThread,
		const FGraphEventRef& MyCompletionGraphEvent) override;
	
}; // struct FFlecsPhaseTickFunction




