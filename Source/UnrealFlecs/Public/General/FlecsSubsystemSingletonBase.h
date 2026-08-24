// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "UObject/ObjectPtr.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsSubsystemSingletonBase.generated.h"

class UFlecsAbstractWorldSubsystem;

USTRUCT(BlueprintInternalUseOnly)
struct UNREALFLECS_API FFlecsSubsystemSingletonBase
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsSubsystemSingletonBase() = default;
	FORCEINLINE FFlecsSubsystemSingletonBase(UFlecsAbstractWorldSubsystem* InSubsystem)
		: Subsystem(InSubsystem)
	{
	}
	
	FORCEINLINE void SetSubsystem(UFlecsAbstractWorldSubsystem* InSubsystem)
	{
		Subsystem = InSubsystem;
	}
	
	NO_DISCARD FORCEINLINE bool IsValid() const
	{
		return Subsystem != nullptr;
	}
	
	NO_DISCARD FORCEINLINE UFlecsAbstractWorldSubsystem* GetSubsystem() const
	{
		return Subsystem;
	}
	
	NO_DISCARD TSolidNotNull<UFlecsAbstractWorldSubsystem*> GetSubsystemChecked() const;

	template <typename T>
	NO_DISCARD FORCEINLINE T* GetSubsystem() const
	{
		return Cast<T>(Subsystem);
	}
	
	template <typename T>
	NO_DISCARD FORCEINLINE TSolidNotNull<T*> GetSubsystemChecked() const
	{
		return CastChecked<T>(GetSubsystemChecked());
	}
	
	FORCEINLINE UFlecsAbstractWorldSubsystem* operator->() const
	{
		return Subsystem;
	}
	
private:
	UPROPERTY()
	TObjectPtr<UFlecsAbstractWorldSubsystem> Subsystem;
	
}; // struct FFlecsSubsystemSingletonBase

template <>
struct TFlecsComponentTraits<FFlecsSubsystemSingletonBase> : public TFlecsComponentTraitsBase<FFlecsSubsystemSingletonBase>
{
	// @TODO: Should we allow inheritance in flecs for this?
	//static constexpr bool Singleton = true;
}; // struct TFlecsComponentTraits<FFlecsSubsystemSingletonBase>

