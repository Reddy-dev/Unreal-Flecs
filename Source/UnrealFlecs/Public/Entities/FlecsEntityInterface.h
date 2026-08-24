// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "Entities/FlecsEntityHandle.h"

#include "FlecsEntityInterface.generated.h"

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UNREALFLECS_API UFlecsEntityInterface : public UInterface
{
	GENERATED_BODY()
}; // class UFlecsEntityInterface

class UNREALFLECS_API IFlecsEntityInterface
{
	GENERATED_BODY()

public:
	NO_DISCARD virtual FFlecsEntityHandle GetEntityHandle() const = 0;

	NO_DISCARD FORCEINLINE bool IsValidEntity() const
	{
		return GetEntityHandle().IsValid();
	}

}; // class IFlecsEntityInterface 
