// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "Entities/FlecsEntityView.h"
#include "Entities/FlecsId.h"
#include "Entities/FlecsComponentRefTypes.h"

#include "FlecsUntypedComponentRefFunctionLibrary.generated.h"

class UScriptStruct;

UCLASS(BlueprintType)
class UNREALFLECS_API UFlecsUntypedComponentRefFunctionLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Flecs | Untyped Component Ref", meta = (BlueprintInternalUseOnly = "true"))
	static FFlecsUntypedComponentRef EntityView_GetUntypedComponentRefId(
		const FFlecsEntityView& Entity,
		const FFlecsId ComponentId);

	UFUNCTION(BlueprintPure, Category = "Flecs | Untyped Component Ref", meta = (BlueprintInternalUseOnly = "true"))
	static FFlecsUntypedComponentRef EntityView_GetUntypedComponentRefScriptStruct(
		const FFlecsEntityView& Entity,
		const UScriptStruct* ScriptStruct);

	UFUNCTION(BlueprintPure, Category = "Flecs | Untyped Component Ref",
		meta = (DisplayName = "Is Valid (Flecs Untyped Component Ref)", BlueprintThreadSafe))
	static bool IsValid_FlecsUntypedComponentRef(const FFlecsUntypedComponentRef& Reference);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Untyped Component Ref",
		meta = (DisplayName = "To Bool (Flecs Untyped Component Ref)", CompactNodeTitle = "->", BlueprintThreadSafe))
	static bool ToBool_FlecsUntypedComponentRef(const FFlecsUntypedComponentRef& Reference);

	UFUNCTION(BlueprintCallable, BlueprintPure, CustomThunk, Category = "Flecs | Untyped Component Ref",
		meta = (DisplayName = "Get Value (Flecs Untyped Component Ref)", CustomStructureParam = "Value"))
	static void UntypedComponentRef_GetValue(const FFlecsUntypedComponentRef& Reference, int32& Value);
	DECLARE_FUNCTION(execUntypedComponentRef_GetValue);

	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Flecs | Untyped Component Ref",
		meta = (DisplayName = "Set Value (Flecs Untyped Component Ref)", CustomStructureParam = "Value"))
	static void UntypedComponentRef_SetValue(const FFlecsUntypedComponentRef& Reference, const int32& Value);
	DECLARE_FUNCTION(execUntypedComponentRef_SetValue);

	UFUNCTION(BlueprintCallable, Category = "Flecs | Untyped Component Ref",
		meta = (DisplayName = "Modified Component (Flecs Untyped Component Ref)"))
	static void UntypedComponentRef_Modified(const FFlecsUntypedComponentRef& Reference);

}; // class UFlecsUntypedComponentRefFunctionLibrary
