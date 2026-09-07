// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once


#include "Engine/EngineBaseTypes.h"
#include "StructUtils/SharedStruct.h"

#include "SolidMacros/Macros.h"
#include "Standard/Hashing.h"

#include "FlecsWorldInfoSettings.generated.h"

struct FFlecsTickFunction;

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsTickFunctionSettingsInfo
{
    GENERATED_BODY()

public:
    FFlecsTickFunctionSettingsInfo();

    UPROPERTY(EditAnywhere)
    TEnumAsByte<ETickingGroup> TickGroup = ETickingGroup::TG_PrePhysics;

    UPROPERTY(EditAnywhere)
    TEnumAsByte<ETickingGroup> EndTickGroup = TickGroup;
    
    UPROPERTY(EditAnywhere)
    uint8 bCanEverTick : 1 = true;

    UPROPERTY(EditAnywhere)
    uint8 bStartWithTickEnabled : 1 = true;

    UPROPERTY(EditAnywhere)
    uint8 bAllowTickOnDedicatedServer : 1 = true;

    UPROPERTY(EditAnywhere)
    uint8 bTickEvenWhenPaused : 1 = false;

    UPROPERTY()
    uint8 bHighPriority : 1 = true;

    UPROPERTY()
    uint8 bAllowTickBatching : 1 = false;

    UPROPERTY()
    uint8 bRunTransactionally : 1 = true;
    
    UPROPERTY(EditAnywhere)
    float TickInterval = 0.0f;

    UPROPERTY(EditAnywhere, AdvancedDisplay, meta = (NoElementDuplicate))
    TArray<FGameplayTag> TickFunctionPrerequisiteTags;
    
    NO_DISCARD static TSharedStruct<FFlecsTickFunction> CreateTickFunctionInstance(const FFlecsTickFunctionSettingsInfo& InTickFunctionSettings);
    
}; // struct FFlecsTickFunctionSettingsInfo

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsWorldSettingsInfo
{
    GENERATED_BODY()

    NO_DISCARD FORCEINLINE friend uint32 GetTypeHash(const FFlecsWorldSettingsInfo& InWorldSettings)
    {
        return GetTypeHash(InWorldSettings.WorldName);
    }
    
public:
    FFlecsWorldSettingsInfo();
    
    FORCEINLINE FFlecsWorldSettingsInfo(const FString& InWorldName, const TArray<TObjectPtr<UObject>>& InGameLoop)
        : WorldName(InWorldName)
        , GameLoops(InGameLoop)
    {
    }

    NO_DISCARD FORCEINLINE bool operator==(const FFlecsWorldSettingsInfo& Other) const
    {
        return WorldName == Other.WorldName;
    }

    NO_DISCARD FORCEINLINE bool operator!=(const FFlecsWorldSettingsInfo& Other) const
    {
        return !(*this == Other);
    }

    UPROPERTY(EditAnywhere, Category = "World")
    FString WorldName;
    
    // @TODO: add FLECS_REST and FLECS_STATS checks
    
    UPROPERTY(EditAnywhere, Category = "World")
    bool bImportRest = true;
    
    UPROPERTY(EditAnywhere, Category = "World")
    bool bImportStats = true;
    
    UPROPERTY(EditAnywhere, Instanced, Category = "Game Loop",
        meta = (ObjectMustImplement = "/Script/UnrealFlecs.FlecsGameLoopInterface", NoElementDuplicate))
    TArray<TObjectPtr<UObject>> GameLoops;
    
}; // struct FFlecsWorldSettingsInfo

DEFINE_STD_HASH(FFlecsWorldSettingsInfo);
