// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/EngineBaseTypes.h"

#include "SolidMacros/Macros.h"
#include "Standard/Hashing.h"
#include "Types/SolidNotNull.h"

#include "StructUtils/InstancedStruct.h"
#include "StructUtils/SharedStruct.h"

#include "FlecsWorldInfoSettings.generated.h"

struct FFlecsTickFunction;

class UFlecsModuleSetDataAsset;

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
    
    FORCEINLINE FFlecsWorldSettingsInfo(const FString& InWorldName, const TArray<TObjectPtr<UObject>>& InGameLoop,
        const TArray<TObjectPtr<UObject>>& InModules = {})
        : WorldName(InWorldName)
        , GameLoops(InGameLoop)
        , Modules(InModules)
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

    UPROPERTY(EditAnywhere, Instanced, Category = "Game Loop",
        meta = (ObjectMustImplement = "/Script/UnrealFlecs.FlecsGameLoopInterface", NoElementDuplicate))
    TArray<TObjectPtr<UObject>> GameLoops;

    UPROPERTY(EditAnywhere, Instanced, Category = "Modules",
        meta = (ObjectMustImplement = "/Script/UnrealFlecs.FlecsModuleInterface", NoElementDuplicate))
    TArray<TObjectPtr<UObject>> Modules;

    UPROPERTY(EditAnywhere, Category = "Modules", meta = (NoElementDuplicate))
    TArray<TObjectPtr<UFlecsModuleSetDataAsset>> ModuleSets;

#if WITH_EDITORONLY_DATA

    UPROPERTY(EditAnywhere, Instanced, Category = "Editor Modules",
         meta = (ObjectMustImplement = "/Script/UnrealFlecs.FlecsModuleInterface", NoElementDuplicate))
    TArray<TObjectPtr<UObject>> EditorModules;

    UPROPERTY(EditAnywhere, Category = "Editor Modules", meta = (NoElementDuplicate))
    TArray<TObjectPtr<UFlecsModuleSetDataAsset>> EditorModuleSets;

#endif // #if WITH_EDITORONLY_DATA

    UPROPERTY(EditAnywhere, Category = "World", AdvancedDisplay)
    TArray<FFlecsTickFunctionSettingsInfo> TickFunctions;
    
}; // struct FFlecsWorldSettingsInfo

DEFINE_STD_HASH(FFlecsWorldSettingsInfo);

