// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "UObject/Object.h"

#include "SolidMacros/Macros.h"

#include "FlecsId.h"

#include "FlecsEntityRange.generated.h"

UCLASS(BlueprintType, NotBlueprintable)
class UNREALFLECS_API UFlecsEntityRange final : public UObject
{
	GENERATED_BODY()
	
public:
	UFlecsEntityRange(const FObjectInitializer& ObjectInitializer);
	
	void SetNativeEntityRange(const ecs_entity_range_t* InRange, const FName& InRangeName);
	void InvalidateNativeEntityRange();
	
	NO_DISCARD FORCEINLINE const ecs_entity_range_t* GetNativeEntityRange() const
	{
		return NativeRange;
	}
	
	FORCEINLINE operator const ecs_entity_range_t*() const
	{
		return GetNativeEntityRange();
	}
	
	NO_DISCARD uint32 GetMinimum() const;
	NO_DISCARD uint32 GetMaximum() const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | Entity Range", meta = (DisplayName = "Get Minimum"))
	int32 K2_GetMinimum() const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | Entity Range", meta = (DisplayName = "Get Maximum"))
	int32 K2_GetMaximum() const;
	
	NO_DISCARD FORCEINLINE TTuple<uint32, uint32> GetRange() const
	{
		return MakeTuple(GetMinimum(), GetMaximum());
	}
	
	NO_DISCARD FORCEINLINE FName GetRangeName() const
	{
		return RangeName;
	}
	
private:
	// @TODO: stuff -Elie
	UFUNCTION()
	FFlecsId GetCurrentId() const;
	
private:
	const ecs_entity_range_t* NativeRange = nullptr;
	
	UPROPERTY()
	FName RangeName;
	
}; // class UFlecsEntityRange
