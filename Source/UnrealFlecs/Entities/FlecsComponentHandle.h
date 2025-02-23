// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FlecsEntityHandle.h"
#include "FlecsComponentHandle.generated.h"

/**
 * Equivalent to flecs::untyped_component and flecs::component<T>
 */
USTRUCT(BlueprintType, meta = (DisableSplitPin))
struct UNREALFLECS_API FFlecsComponentHandle : public FFlecsEntityHandle
{
	GENERATED_BODY()

public:
	FFlecsComponentHandle() = default;
	
	FFlecsComponentHandle(const FFlecsEntityHandle& InEntityHandle)
		: FFlecsEntityHandle(InEntityHandle)
	{
		solid_checkf(IsComponent(), TEXT("Entity is not a component"));
	}

	FORCEINLINE NO_DISCARD flecs::untyped_component GetUntypedComponent() const
	{
		return GetUntypedComponent_Unsafe();
	}

	template <typename T>
	FORCEINLINE NO_DISCARD flecs::component<T> GetTypedComponent() const
	{
		return flecs::component<T>(GetFlecsWorld_Internal(), GetEntity());
	}
	

	FORCEINLINE operator flecs::untyped_component() const
	{
		return GetUntypedComponent();
	}

	template <typename T>
	FORCEINLINE operator flecs::component<T>() const
	{
		return GetTypedComponent<T>();
	}

	template <typename T>
	FORCEINLINE void AddBit(const FString& InName, const T InValue) const
	{
		GetUntypedComponent_Unsafe().bit<T>(StringCast<char>(*InName).Get(), InValue);
	}
	
}; // struct FFlecsComponentHandle
