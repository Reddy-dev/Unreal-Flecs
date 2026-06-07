// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Entities/FlecsId.h"

#include "FlecsIdBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS(Experimental)
class UNREALFLECS_API UFlecsIdBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	// @TODO: Handle Errored Inputs

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Id", meta = (NativeMakeFunc, BlueprintThreadSafe))
	static FFlecsId MakeFlecsId(const int32 Index, const int32 Generation = 0);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Id", meta = (NativeBreakFunc, BlueprintThreadSafe))
	static void BreakFlecsId(const FFlecsId Id, int32& OutIndex, int32& OutGeneration);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Id", 
		meta = (ScriptMethod = "IsValid", ScriptOperator = "bool", BlueprintThreadSafe))
	static FORCEINLINE bool IsValidFlecsId(const FFlecsId Id)
	{
		return Id.IsValid();
	}
	
	/**
	 * @brief Returns the index(first) part of the Id. MUST NOT BE A PAIR ID.
	 * @param Id The Id to get the index from.
	 * @return The index part of the Id.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Id", meta = (BlueprintThreadSafe))
	static FORCEINLINE int32 GetIdIndex(const FFlecsId Id)
	{
		return Id.GetIndex();
	}
	
	/**
	 * @brief Returns the generation(second) part of the Id.
	 * This is the part that is used to identify the generation of the component, tag,
	 * @param Id 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Id", meta = (BlueprintThreadSafe))
	static FORCEINLINE int32 GetIdGeneration(const FFlecsId Id)
	{
		return Id.GetGeneration();
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Id", meta = (NativeMakeFunc, BlueprintThreadSafe))
	static FFlecsId MakePairId(const FFlecsId First, const FFlecsId Second);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Id", meta = (NativeBreakFunc, BlueprintThreadSafe))
	static bool BreakPairId(const FFlecsId PairId, FFlecsId& OutFirst, FFlecsId& OutSecond);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Id", meta = (BlueprintThreadSafe))
	static FORCEINLINE bool IsPairId(const FFlecsId Id)
	{
		return Id.IsPair();
	}
	
#pragma region Operators
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Id", 
		meta = (DisplayName = "Equal (FlecsId)", CompactNodeTitle = "==", ScriptOperator = "==", BlueprintThreadSafe))
	static bool EqualEqual_FlecsId(const FFlecsId A, const FFlecsId B);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Id", 
		meta = (DisplayName = "Not Equal (FlecsId)", CompactNodeTitle = "!=", ScriptOperator = "!=", BlueprintThreadSafe))
	static bool NotEqual_FlecsId(const FFlecsId A, const FFlecsId B);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Id", 
		meta = (DisplayName = "To String (FlecsId)", CompactNodeTitle = "->", BlueprintThreadSafe))
	static FString ToString_FlecsId(const FFlecsId Id);
	
	
#pragma endregion // Operators
	
	
}; // class UFlecsIdBlueprintFunctionLibrary
