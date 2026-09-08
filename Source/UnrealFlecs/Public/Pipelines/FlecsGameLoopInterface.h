// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "Engine/EngineBaseTypes.h"

#include "Types/SolidNotNull.h"

#include "Entities/FlecsEntityInterface.h"
#include "TickFunctions/FlecsTickFunction.h"

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
	
	// @TODO: Currently unused
	virtual void DeinitializeGameLoop(TSolidNotNull<UFlecsWorld*> InWorld, const FFlecsEntityHandle& InGameLoopEntity) {}
	
	virtual bool Progress(double DeltaTime, 
		TSolidNotNull<UFlecsWorld*> InWorld, 
		ELevelTick InTickType,
		ENamedThreads::Type InCurrentThread,
		const FGraphEventRef& InCompletionGraphEvent)
		PURE_VIRTUAL(IFlecsGameLoopInterface::Progress, return false;)

	virtual bool IsMainLoop() const;
	
	virtual TSharedStruct<FFlecsTickFunction> InitializeTickFunction(TSolidNotNull<UFlecsWorld*> InWorld)
	{
		return TSharedStruct<FFlecsTickFunction>();
	}
	
	virtual NO_DISCARD TSharedStruct<FFlecsTickFunction> GetTickFunction() const
	{
		return TSharedStruct<FFlecsTickFunction>();
	}
	
	virtual NO_DISCARD FFlecsEntityHandle GetEntityHandle() const override
	{
		return GameLoopEntity;
	}
	
	FFlecsEntityHandle GameLoopEntity;
	
}; // class IFlecsGameLoopInterface
