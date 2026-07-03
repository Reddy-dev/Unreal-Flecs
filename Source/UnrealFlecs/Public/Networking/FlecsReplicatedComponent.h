// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsReplicatedComponent.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicatedComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs")
	bool bDeterministicId = false;
	
}; // struct FFlecsReplicatedComponent

