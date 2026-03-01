// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Interfaces/FlecsEntityInterface.h"
#include "FlecsSystemHandle.h"

#include "FlecsSystemHandleInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UFlecsSystemHandleInterface : public UFlecsEntityInterface
{
	GENERATED_BODY()
}; // class UFlecsSystemHandleInterface

/**
 * 
 */
class UNREALFLECS_API IFlecsSystemHandleInterface : public IFlecsEntityInterface
{
	GENERATED_BODY()

public:
	virtual NO_DISCARD FFlecsEntityHandle GetEntityHandle() const override final
	{
		return GetSystemHandle();
	}
	
	virtual NO_DISCARD FFlecsSystemHandle GetSystemHandle() const = 0;

}; // class IFlecsSystemHandleInterface
