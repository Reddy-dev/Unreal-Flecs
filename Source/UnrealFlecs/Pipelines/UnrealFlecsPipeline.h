// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UnrealFlecsPipeline.generated.h"

class UFlecsWorld;

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsPipeline
{
	GENERATED_BODY()

public:

private:
	UPROPERTY()
	TWeakObjectPtr<UFlecsWorld> World;
	
}; // struct FFlecsPipeline

