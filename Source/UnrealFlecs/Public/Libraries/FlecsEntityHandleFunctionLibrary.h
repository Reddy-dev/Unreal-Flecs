// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Entities/FlecsEntityHandle.h"

#include "FlecsEntityHandleFunctionLibrary.generated.h"

UCLASS(BlueprintType)
class UNREALFLECS_API UFlecsEntityHandleFunctionLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity")
    static void DestroyEntity(const FFlecsEntityHandle& Entity);
    
    

}; // class UEntityFunctionLibrary
