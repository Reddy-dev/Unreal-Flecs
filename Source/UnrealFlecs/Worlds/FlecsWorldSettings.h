﻿// Solstice Games © 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SolidMacros/Macros.h"
#include "Standard/Hashing.h"
#include "FlecsWorldSettings.generated.h"

USTRUCT(BlueprintType)
struct FFlecsWorldSettings
{
    GENERATED_BODY()

    FORCEINLINE friend NO_DISCARD uint32 GetTypeHash(const FFlecsWorldSettings& InWorldSettings)
    {
        return GetTypeHash(InWorldSettings.WorldName);
    }
    
public:
    FFlecsWorldSettings() = default;
    
    FORCEINLINE FFlecsWorldSettings(const FString& InWorldName, const bool bInAutoTickWorld = true,
        const bool bInDefaultAutoMerge = true, const int32 InDefaultWorkerThreads = 4,
        const TArray<TObjectPtr<UObject>>& InModules = {})
        : WorldName(InWorldName)
        , bAutoTickWorld(bInAutoTickWorld)
        , DefaultWorkerThreads(InDefaultWorkerThreads)
        , Modules(InModules)
    {
    }

    FORCEINLINE NO_DISCARD bool operator==(const FFlecsWorldSettings& Other) const
    {
        return WorldName == Other.WorldName;
    }

    FORCEINLINE NO_DISCARD bool operator!=(const FFlecsWorldSettings& Other) const
    {
        return !(*this == Other);
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs")
    FString WorldName;
    
    UPROPERTY(EditAnywhere, Category = "Flecs")
    uint8 bAutoTickWorld : 1 = true;
    
    UPROPERTY(EditAnywhere, Category = "Flecs")
    uint8 bUseTaskThreads : 1 = true;

    UPROPERTY(EditAnywhere, Category = "Flecs", meta = (ClampMin = "1"))
    int32 DefaultWorkerThreads = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Flecs",
        meta = (MustImplement = "/Script/UnrealFlecs.FlecsModuleInterface"))
    TArray<TObjectPtr<UObject>> Modules;
    
}; // struct FFlecsWorldSettings

DEFINE_STD_HASH(FFlecsWorldSettings);

