// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "Engine/World.h"

#include "SolidMacros/Macros.h"

#include "Properties/FlecsComponentProperties.h"

#include "UWorldPtrComponent.generated.h"

class UWorld;

/**
 * @brief A Singleton Component containing a pointer to the owning UWorld.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FUWorldPtrComponent
{
	GENERATED_BODY()

public:
	FORCEINLINE FUWorldPtrComponent() = default;
	
	FORCEINLINE FUWorldPtrComponent(UWorld* InWorld)
		: World(InWorld)
	{
	}
	
	FORCEINLINE void SetWorld(UWorld* InWorld)
	{
		World = InWorld;
	}
	
	NO_DISCARD FORCEINLINE UWorld* GetWorld() const
	{
		return World;
	}
	
	NO_DISCARD FORCEINLINE bool IsValid() const
	{
		return ::IsValid(World);
	}

	FORCEINLINE FUWorldPtrComponent& operator=(UWorld* InWorld)
	{
		World = InWorld;
		return *this;
	}

	NO_DISCARD FORCEINLINE bool UEOpEquals(const FUWorldPtrComponent& InComponent) const
	{
		return World == InComponent.World;
	}

	NO_DISCARD FORCEINLINE bool UEOpEquals(const UWorld* InWorld) const
	{
		return World == InWorld;
	}

	NO_DISCARD FORCEINLINE bool UEOpEquals(const TWeakObjectPtr<UWorld>& InWorld) const
	{
		return World == InWorld;
	}

	UPROPERTY(BlueprintReadOnly, Category = "Flecs")
	TObjectPtr<UWorld> World;
	
}; // struct FUWorldPtrComponent

template <>
struct TFlecsComponentTraits<FUWorldPtrComponent> : public TFlecsComponentTraitsBase<FUWorldPtrComponent>
{
	static constexpr bool AutoRegister = false;
	
	static constexpr bool UseLowId = false;
}; // struct TFlecsComponentTraits<FUWorldPtrComponent>

template<>
struct TStructOpsTypeTraits<FUWorldPtrComponent> : public TStructOpsTypeTraitsBase2<FUWorldPtrComponent>
{
	enum
	{
		
	}; // enum
	
}; // struct TStructOpsTypeTraits<FUWorldPtrComponent>
