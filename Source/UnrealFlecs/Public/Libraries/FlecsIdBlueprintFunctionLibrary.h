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
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Id", 
		meta = (ScriptMethod = "IsValid", ScriptOperator = "bool", BlueprintThreadSafe))
	static FORCEINLINE bool IsValidFlecsId(const FFlecsId Id)
	{
		return Id.IsValid();
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Id", meta = (BlueprintThreadSafe))
	static FORCEINLINE int32 GetIdIndex(const FFlecsId Id)
	{
		return Id.GetIndex();
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Id", meta = (BlueprintThreadSafe))
	static FORCEINLINE int32 GetIdGeneration(const FFlecsId Id)
	{
		return Id.GetGeneration();
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Id", meta = (BlueprintThreadSafe))
	static FFlecsId MakePairId(const FFlecsId First, const FFlecsId Second);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs | Id", meta = (BlueprintThreadSafe))
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
