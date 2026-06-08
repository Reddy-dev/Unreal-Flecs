// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Entities/FlecsEntityView.h"
#include "Entities/FlecsEntityHandle.h"

#include "FlecsEntityHandleFunctionLibrary.generated.h"

UCLASS(BlueprintType)
class UNREALFLECS_API UFlecsEntityHandleFunctionLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", 
        meta = (DisplayName = "To Bool (Flecs Entity View)", CompactNodeTitle = "->", BlueprintThreadSafe))
    static bool ToBool_FlecsEntityView(const FFlecsEntityView& Id);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", 
        meta = (DisplayName = "Is Valid (Flecs Entity View)", CompactNodeTitle = "Is Valid", BlueprintThreadSafe))
    static bool IsValid_FlecsEntityView(const FFlecsEntityView& Id);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", 
       meta = (DisplayName = "Is Alive (Flecs Entity View)", BlueprintThreadSafe))
    static bool IsAlive_FlecsEntityView(const FFlecsEntityView& Id);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", 
        meta = (DisplayName = "To Flecs Id (Flecs Entity View)", CompactNodeTitle = "->", BlueprintThreadSafe))
    static FFlecsId ToFlecsId_FlecsEntityView(const FFlecsEntityView& Id);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintThreadSafe))
    static bool IsUnrealFlecsWorld(const FFlecsEntityView& Id);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintThreadSafe))
    static bool IsInStage(const FFlecsEntityView& Id);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintThreadSafe))
    static FFlecsId GetFlecsId(const FFlecsEntityView& Entity);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintThreadSafe))
    static UFlecsWorldInterfaceObject* GetFlecsWorld(const FFlecsEntityView& Entity);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintThreadSafe))
    static UWorld* GetOuterWorld(const FFlecsEntityView& Entity);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", 
        meta = (DisplayName = "To String (Flecs Entity View)", CompactNodeTitle = "->", BlueprintThreadSafe))
    static FString ToString_FlecsEntityView(const FFlecsEntityView& Id);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", 
        meta = (DisplayName = "Equal (Flecs Entity View)", CompactNodeTitle = "==", ScriptOperator = "==", BlueprintThreadSafe))
    static bool EqualEqual_FlecsEntityView(const FFlecsEntityView& A, const FFlecsEntityView& B);
	
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", 
        meta = (DisplayName = "Not Equal (Flecs Entity View)", CompactNodeTitle = "!=", ScriptOperator = "!=", BlueprintThreadSafe))
    static bool NotEqual_FlecsEntityView(const FFlecsEntityView& A, const FFlecsEntityView& B);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", 
        meta = (DisplayName = "Equal (Flecs Entity Handle)", CompactNodeTitle = "==", ScriptOperator = "==", BlueprintThreadSafe))
    static bool EqualEqual_FlecsEntityHandle(const FFlecsEntityHandle& A, const FFlecsEntityHandle& B);
	
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", 
        meta = (DisplayName = "Not Equal (Flecs Entity Handle)", CompactNodeTitle = "!=", ScriptOperator = "!=", BlueprintThreadSafe))
    static bool NotEqual_FlecsEntityHandle(const FFlecsEntityHandle& A, const FFlecsEntityHandle& B);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", 
        meta = (DisplayName = "To Flecs Entity View (Flecs Entity Handle)", CompactNodeTitle = "->", BlueprintAutocast, BlueprintThreadSafe))
    static FFlecsEntityView Conv_FlecsEntityHandleToView(const FFlecsEntityHandle& Entity);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", 
        meta = (DisplayName = "To Flecs Id (Flecs Entity View)", CompactNodeTitle = "->", BlueprintAutocast, BlueprintThreadSafe))
    static FFlecsId Conv_FlecsEntityViewToId(const FFlecsEntityView& Entity);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", 
        meta = (DisplayName = "To Flecs Id (Flecs Entity Handle)", CompactNodeTitle = "->", BlueprintAutocast, BlueprintThreadSafe))
    static FFlecsId Conv_FlecsEntityHandleToId(const FFlecsEntityHandle& Entity);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", 
        meta = (DisplayName = "To Mut (Flecs Entity View)", CompactNodeTitle = "To Mut", BlueprintThreadSafe))
    static FFlecsEntityHandle ToMut_FlecsEntityViewWithWorld(const FFlecsEntityView& Entity, const UFlecsWorldInterfaceObject* World);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintThreadSafe))
    static bool IsComponent(const FFlecsEntityView& Entity);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintThreadSafe))
    static bool IsTag(const FFlecsEntityView& Entity);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintThreadSafe))
    static bool IsValueComponent(const FFlecsEntityView& Entity);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintThreadSafe))
    static bool IsEnum(const FFlecsEntityView& Entity);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintThreadSafe))
    static bool HasParent(const FFlecsEntityView& Entity);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintThreadSafe))
    static FFlecsEntityHandle GetParent(const FFlecsEntityView& Entity);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintThreadSafe))
    static bool IsPrefab(const FFlecsEntityView& Entity);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintThreadSafe))
    static bool IsEnabled(const FFlecsEntityView& Entity);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintThreadSafe))
    static bool HasName(const FFlecsEntityView& Entity);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintThreadSafe))
    static FString GetName(const FFlecsEntityView& Entity);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity",
        meta = (BlueprintThreadSafe, AdvancedDisplay = "1"))
    static FString GetPath(const FFlecsEntityView& Entity, const FString& Separator = "::", const FString& RootSeparator = "::");

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity")
    static void DestroyEntity(const FFlecsEntityHandle& Entity);
    
    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity")
    static void ClearEntity(const FFlecsEntityHandle& Entity);
    
    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity")
    static void EnableEntity(const FFlecsEntityHandle& Entity);
    
    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity")
    static void DisableEntity(const FFlecsEntityHandle& Entity);
    
    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity")
    static bool ToggleEntity(const FFlecsEntityHandle& Entity);
    
    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity")
    static void SetName(const FFlecsEntityHandle& Entity, const FString& Name);
    
    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity")
    static void SetParent(const FFlecsEntityHandle& Entity, const FFlecsEntityHandle& Parent, const bool bDontFragment = false);

}; // class UEntityFunctionLibrary
