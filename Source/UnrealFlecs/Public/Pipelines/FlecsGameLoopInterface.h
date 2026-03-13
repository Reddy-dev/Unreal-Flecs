// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Types/SolidNotNull.h"

#include "Interfaces/FlecsEntityInterface.h"

#include "FlecsGameLoopInterface.generated.h"

class UFlecsWorld;

// This class does not need to be modified.
UINTERFACE(meta = (CannotImplementInterfaceInBlueprint))
class UNREALFLECS_API UFlecsGameLoopInterface : public UFlecsEntityInterface
{
	GENERATED_BODY()
}; // class UFlecsGameLoopInterface

class UNREALFLECS_API IFlecsGameLoopInterface : public IFlecsEntityInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	void InitializeGameLoop_Internal(TSolidNotNull<UFlecsWorld*> InWorld);
	
	virtual void InitializeGameLoop(TSolidNotNull<UFlecsWorld*> InWorld, const FFlecsEntityHandle& InGameLoopEntity) {}
	
	virtual bool Progress(double DeltaTime, const FGameplayTag& InTickType, TSolidNotNull<UFlecsWorld*> InWorld)
		PURE_VIRTUAL(IFlecsGameLoopInterface::Progress, return false;)

	virtual bool IsMainLoop() const;
	
	virtual NO_DISCARD FFlecsEntityHandle GetEntityHandle() const override final
	{
		return GameLoopEntity;
	}

	virtual TArray<FGameplayTag> GetTickTypeTags() const;
	
	FFlecsEntityHandle GameLoopEntity;
	
}; // class IFlecsGameLoopInterface
