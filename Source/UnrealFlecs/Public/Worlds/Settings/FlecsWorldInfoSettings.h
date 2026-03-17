// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

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

    NO_DISCARD FORCEINLINE friend uint32 GetTypeHash(const FFlecsTickFunctionSettingsInfo& InTickFunctionSettings)
    {
        return GetTypeHash(InTickFunctionSettings.TickTypeTag);
    }

public:
    FFlecsTickFunctionSettingsInfo();
    
    UPROPERTY(EditAnywhere)
    FString TickFunctionName;

    UPROPERTY(EditAnywhere)
    FGameplayTag TickTypeTag;

    UPROPERTY(EditAnywhere)
    TEnumAsByte<ETickingGroup> TickGroup = ETickingGroup::TG_PrePhysics;

    UPROPERTY(EditAnywhere)
    TEnumAsByte<ETickingGroup> EndTickGroup = TickGroup;

    UPROPERTY(EditAnywhere)
    bool bStartWithTickEnabled = true;

    UPROPERTY(EditAnywhere)
    bool bAllowTickOnDedicatedServer = true;

    UPROPERTY(EditAnywhere)
    bool bTickEvenWhenPaused = false;

    UPROPERTY(EditAnywhere)
    float TickInterval = 0.0f;

    UPROPERTY()
    bool bHighPriority = true;

    UPROPERTY()
    bool bAllowTickBatching = false;

    UPROPERTY()
    bool bRunTransactionally = true;

    UPROPERTY(EditAnywhere, AdvancedDisplay, meta = (NoElementDuplicate))
    TArray<FGameplayTag> TickFunctionPrerequisiteTags;

    static NO_DISCARD FFlecsTickFunctionSettingsInfo GetTickFunctionSettingsDefault(const FGameplayTag& InTickTypeTag);

    static NO_DISCARD TSharedStruct<FFlecsTickFunction> CreateTickFunctionInstance(
        const FFlecsTickFunctionSettingsInfo& InTickFunctionSettings);
    
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

    UPROPERTY(EditAnywhere, Category = "World", AdvancedDisplay)
    TArray<FFlecsTickFunctionSettingsInfo> TickFunctions;
    
}; // struct FFlecsWorldSettingsInfo

DEFINE_STD_HASH(FFlecsWorldSettingsInfo);

