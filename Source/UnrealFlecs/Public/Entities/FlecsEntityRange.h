// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "UObject/Object.h"

#include "SolidMacros/Macros.h"

#include "FlecsId.h"

#include "FlecsEntityRange.generated.h"

/**
 * @brief UObject wrapper around a native Flecs entity-id range.
 *
 * UFlecsWorld owns the native range and its lifetime. The wrapper exposes the
 * inclusive minimum/maximum bounds and the world-managed range name to C++
 * and Blueprint consumers. Its native pointer becomes invalid when the owning
 * world resets or is destroyed.
 *
 * @see UFlecsWorld::CreateEntityRange()
 * @see UFlecsWorld::SetActiveEntityRange()
 */
UCLASS(BlueprintType, NotBlueprintable)
class UNREALFLECS_API UFlecsEntityRange final : public UObject
{
	GENERATED_BODY()
	
	friend class UFlecsWorld;
	
public:
	/**
	 * @brief Constructs a range wrapper owned by UFlecsWorld.
	 * @param ObjectInitializer Unreal object initialization data.
	 */
	UFlecsEntityRange(const FObjectInitializer& ObjectInitializer);
	
	/**
	 * @brief Invalidates the wrapped native range pointer.
	 *
	 * Called by the owning world during reset and teardown. The wrapper must
	 * not be used for range queries after invalidation.
	 */
	void InvalidateNativeEntityRange();
	
	/**
	 * @brief Returns the wrapped native Flecs range.
	 * @return The native range, or nullptr after world teardown/reset.
	 */
	NO_DISCARD FORCEINLINE const ecs_entity_range_t* GetNativeEntityRange() const
	{
		return NativeRange;
	}
	
	/** Converts this wrapper to its native Flecs range pointer. */
	FORCEINLINE operator const ecs_entity_range_t*() const
	{
		return GetNativeEntityRange();
	}
	
	/**
	 * @brief Returns the first entity id in the range.
	 * @return Inclusive minimum id.
	 * @warning The native range must still be valid.
	 */
	NO_DISCARD uint32 GetMinimum() const;
	/**
	 * @brief Returns the last entity id in the range.
	 * @return Inclusive maximum id; zero means unbounded.
	 * @warning The native range must still be valid.
	 */
	NO_DISCARD uint32 GetMaximum() const;
	
	/** Blueprint wrapper for GetMinimum(). */
	UFUNCTION(BlueprintCallable, Category = "Flecs | Entity Range", meta = (DisplayName = "Get Minimum"))
	int32 K2_GetMinimum() const;
	
	/** Blueprint wrapper for GetMaximum(). */
	UFUNCTION(BlueprintCallable, Category = "Flecs | Entity Range", meta = (DisplayName = "Get Maximum"))
	int32 K2_GetMaximum() const;
	
	/**
	 * @brief Returns both inclusive range bounds.
	 * @return Tuple containing minimum and maximum; maximum zero is unbounded.
	 */
	NO_DISCARD FORCEINLINE TTuple<uint32, uint32> GetRange() const
	{
		return MakeTuple(GetMinimum(), GetMaximum());
	}
	
	/** Returns the name assigned when the world created this range. */
	UFUNCTION(BlueprintCallable, Category = "Flecs | Entity Range")
	FORCEINLINE FName GetRangeName() const
	{
		return RangeName;
	}
	
private:
	void SetNativeEntityRange(const TSolidNotNull<const ecs_entity_range_t*> InRange, const FName& InRangeName);
	
private:
	// @TODO: stuff -Elie
	UFUNCTION()
	FFlecsId GetCurrentId() const;
	
private:
	const ecs_entity_range_t* NativeRange = nullptr;
	
	UPROPERTY()
	FName RangeName;
	
}; // class UFlecsEntityRange
