// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Entities/FlecsEntityView.h"
#include "Entities/FlecsEntityHandle.h"
#include "Entities/FlecsComponentRef.h"

#include "FlecsEntityHandleFunctionLibrary.generated.h"

UCLASS(BlueprintType)
class UNREALFLECS_API UFlecsEntityHandleFunctionLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", 
       meta = (NativeMakeFunc, BlueprintThreadSafe))
    static FFlecsEntityView MakeFlecsEntityViewHandle(const FFlecsId Id, const UFlecsWorldInterfaceObject* World);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", 
        meta = (NativeMakeFunc, BlueprintThreadSafe))
    static FFlecsEntityHandle MakeFlecsEntityHandle(const FFlecsId Id, const UFlecsWorldInterfaceObject* World);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", 
        meta = (DisplayName = "To Bool (Flecs Entity View)", CompactNodeTitle = "->", BlueprintThreadSafe))
    static bool ToBool_FlecsEntityView(const FFlecsEntityView& Id);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", 
        meta = (DisplayName = "Is Valid Entity", CompactNodeTitle = "Is Valid", BlueprintThreadSafe))
    static bool IsValid_FlecsEntityView(const FFlecsEntityView& Id);
    
    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity", 
        meta = (DisplayName = "Is Valid Entity (Branch)", BlueprintThreadSafe, ExpandBoolAsExecs = "ReturnValue"))
    static bool IsValidBranch_FlecsEntityView(const FFlecsEntityView& Id);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", 
       meta = (DisplayName = "Is Alive Entity", BlueprintThreadSafe))
    static bool IsAlive_FlecsEntityView(const FFlecsEntityView& Id);
    
    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity", 
        meta = (DisplayName = "Is Alive Entity (Branch)", BlueprintThreadSafe, ExpandBoolAsExecs = "ReturnValue"))
    static bool IsAliveBranch_FlecsEntityView(const FFlecsEntityView& Id);
    
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
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintThreadSafe, AdvancedDisplay = "1"))
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

#pragma region Operations

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static void EntityHandle_AddId(const FFlecsEntityHandle& Entity, const FFlecsId ComponentId);

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static void EntityHandle_AddScriptStruct(const FFlecsEntityHandle& Entity, const UScriptStruct* ScriptStruct);

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static void EntityHandle_AddEnum(const FFlecsEntityHandle& Entity, const UEnum* Enum);

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static void EntityHandle_AddEnumConstant(const FFlecsEntityHandle& Entity, FSolidEnumSelector EnumConstant);

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static void EntityHandle_AddGameplayTag(const FFlecsEntityHandle& Entity, FGameplayTag GameplayTag);

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static void EntityHandle_AddSolidEnumSelector(const FFlecsEntityHandle& Entity, FSolidEnumSelector EnumSelector);

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static void EntityHandle_AddPairIds(const FFlecsEntityHandle& Entity, const FFlecsId RelationId, const FFlecsId TargetId);

    UFUNCTION(BlueprintCallable, CustomThunk, Category = "Flecs | Entity",
        meta = (BlueprintInternalUseOnly = "true", CustomStructureParam = "Value"))
    static void EntityHandle_SetId(const FFlecsEntityHandle& Entity, const FFlecsId ComponentId, const int32& Value);
    DECLARE_FUNCTION(execEntityHandle_SetId);

    UFUNCTION(BlueprintCallable, CustomThunk, Category = "Flecs | Entity",
        meta = (BlueprintInternalUseOnly = "true", CustomStructureParam = "Value"))
    static void EntityHandle_SetSolidEnumSelector(
        const FFlecsEntityHandle& Entity,
        FSolidEnumSelector EnumSelector,
        const int32& Value);
    DECLARE_FUNCTION(execEntityHandle_SetSolidEnumSelector);

    UFUNCTION(BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static FFlecsId EntityHandle_MakePairId(const FFlecsId RelationId, const FFlecsId TargetId);

    UFUNCTION(BlueprintCallable, CustomThunk, BlueprintPure, Category = "Flecs | Entity",
        meta = (BlueprintInternalUseOnly = "true", CustomStructureParam = "Value"))
    static void EntityView_GetId(const FFlecsEntityView& Entity, const FFlecsId ComponentId, int32& Value);
    DECLARE_FUNCTION(execEntityView_GetId);

    UFUNCTION(BlueprintCallable, CustomThunk, BlueprintPure, Category = "Flecs | Entity",
        meta = (BlueprintInternalUseOnly = "true", CustomStructureParam = "Value"))
    static void EntityView_GetScriptStruct(
        const FFlecsEntityView& Entity,
        const UScriptStruct* ScriptStruct,
        int32& Value);
    DECLARE_FUNCTION(execEntityView_GetScriptStruct);

    UFUNCTION(BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static FFlecsComponentRef EntityView_GetRefId(const FFlecsEntityView& Entity, const FFlecsId ComponentId);

    UFUNCTION(BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static FFlecsComponentRef EntityView_GetRefScriptStruct(
        const FFlecsEntityView& Entity,
        const UScriptStruct* ScriptStruct);

    UFUNCTION(BlueprintPure, Category = "Flecs | Component Reference")
    static bool IsValid_FlecsComponentRef(const FFlecsComponentRef& Reference);

    UFUNCTION(BlueprintCallable, CustomThunk, BlueprintPure, Category = "Flecs | Component Reference",
        meta = (CustomStructureParam = "Value"))
    static void ComponentRef_GetValue(const FFlecsComponentRef& Reference, int32& Value);
    DECLARE_FUNCTION(execComponentRef_GetValue);

    UFUNCTION(BlueprintCallable, CustomThunk, Category = "Flecs | Component Reference",
        meta = (CustomStructureParam = "Value"))
    static void ComponentRef_SetValue(const FFlecsComponentRef& Reference, const int32& Value);
    DECLARE_FUNCTION(execComponentRef_SetValue);

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static void EntityHandle_RemoveId(const FFlecsEntityHandle& Entity, const FFlecsId ComponentId);

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static void EntityHandle_RemoveScriptStruct(const FFlecsEntityHandle& Entity, const UScriptStruct* ScriptStruct);

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static void EntityHandle_RemoveEnum(const FFlecsEntityHandle& Entity, const UEnum* Enum);

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static void EntityHandle_RemoveEnumConstant(const FFlecsEntityHandle& Entity, FSolidEnumSelector EnumConstant);

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static void EntityHandle_RemoveGameplayTag(const FFlecsEntityHandle& Entity, FGameplayTag GameplayTag);

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static void EntityHandle_RemoveSolidEnumSelector(const FFlecsEntityHandle& Entity, FSolidEnumSelector EnumSelector);

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static void EntityHandle_ModifiedId(const FFlecsEntityHandle& Entity, const FFlecsId ComponentId);

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static void EntityHandle_ModifiedScriptStruct(const FFlecsEntityHandle& Entity, const UScriptStruct* ScriptStruct);

    UFUNCTION(BlueprintCallable, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static void EntityHandle_ModifiedPairIds(
        const FFlecsEntityHandle& Entity,
        const FFlecsId RelationId,
        const FFlecsId TargetId);

    UFUNCTION(BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static bool EntityView_HasId(const FFlecsEntityView& Entity, const FFlecsId ComponentId);

    UFUNCTION(BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static bool EntityView_HasScriptStruct(const FFlecsEntityView& Entity, const UScriptStruct* ScriptStruct);

    UFUNCTION(BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static bool EntityView_HasEnum(const FFlecsEntityView& Entity, const UEnum* Enum);

    UFUNCTION(BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static bool EntityView_HasEnumConstant(const FFlecsEntityView& Entity, FSolidEnumSelector EnumConstant);

    UFUNCTION(BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static bool EntityView_HasGameplayTag(const FFlecsEntityView& Entity, FGameplayTag GameplayTag);

    UFUNCTION(BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static bool EntityView_HasSolidEnumSelector(
        const FFlecsEntityView& Entity,
        FSolidEnumSelector EnumSelector);

    UFUNCTION(BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static bool EntityView_HasPairIds(const FFlecsEntityView& Entity, const FFlecsId RelationId, const FFlecsId TargetId);

    UFUNCTION(BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static bool EntityView_HasAnyIds(const FFlecsEntityView& Entity, const TArray<FFlecsId>& ComponentIds);

    UFUNCTION(BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static bool EntityView_HasAllIds(const FFlecsEntityView& Entity, const TArray<FFlecsId>& ComponentIds);

    UFUNCTION(BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static FFlecsId EntityView_ResolveId(const FFlecsEntityView& Entity, const FFlecsId Id);

    UFUNCTION(BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static FFlecsId EntityView_ResolveScriptStructId(const FFlecsEntityView& Entity, const UScriptStruct* ScriptStruct);

    UFUNCTION(BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static FFlecsId EntityView_ResolveEnumId(const FFlecsEntityView& Entity, const UEnum* Enum);

    UFUNCTION(BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static FFlecsId EntityView_ResolveEnumConstantId(const FFlecsEntityView& Entity, FSolidEnumSelector EnumConstant);

    UFUNCTION(BlueprintPure, Category = "Flecs | Entity", meta = (BlueprintInternalUseOnly = "true"))
    static FFlecsId EntityView_ResolveGameplayTagId(const FFlecsEntityView& Entity, FGameplayTag GameplayTag);

#pragma endregion Operation

}; // class UEntityFunctionLibrary
