// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Interfaces/FlecsEntityInterface.h"
#include "FlecsObserverHandle.h"

#include "FlecsObserverHandleInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UFlecsObserverHandleInterface : public UFlecsEntityInterface
{
	GENERATED_BODY()
}; // UFlecsObserverHandleInterface

class UNREALFLECS_API IFlecsObserverHandleInterface : public IFlecsEntityInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual NO_DISCARD FFlecsObserverHandle GetObserverHandle() const
		PURE_VIRTUAL(IFlecsObserverHandleInterface::GetObserverHandle, return FFlecsObserverHandle(););
	
	virtual NO_DISCARD FFlecsEntityHandle GetEntityHandle() const override final;

}; // IFlecsObserverHandleInterface
