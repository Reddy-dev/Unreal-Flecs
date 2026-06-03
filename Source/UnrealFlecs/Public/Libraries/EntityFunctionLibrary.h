// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Entities/FlecsEntityHandle.h"

#include "EntityFunctionLibrary.generated.h"

UCLASS(BlueprintType)
class UNREALFLECS_API UEntityFunctionLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity")
    static void DestroyEntity(const FFlecsEntityHandle& Entity);

}; // class UEntityFunctionLibrary
