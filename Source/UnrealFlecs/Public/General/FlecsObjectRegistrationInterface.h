// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "Types/SolidNotNull.h"

#include "FlecsObjectRegistrationInterface.generated.h"

class UFlecsWorld;

// This class does not need to be modified.
UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UFlecsObjectRegistrationInterface : public UInterface
{
	GENERATED_BODY()
}; // class UFlecsObjectRegistrationInterface

class UNREALFLECS_API IFlecsObjectRegistrationInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void RegisterObject(const TSolidNotNull<UFlecsWorld*> InFlecsWorld);
	virtual void UnregisterObject(const TSolidNotNull<UFlecsWorld*> InFlecsWorld);
	virtual void FlecsWorldBeginPlay(const TSolidNotNull<UFlecsWorld*> InFlecsWorld);
	
	// Impl must be safe to call on the CDO
	virtual NO_DISCARD bool ShouldAutoRegisterFromCDO() const { return true; }
	virtual NO_DISCARD bool ShouldRegisterWithModule() const { return true; }
	
	// Optional, if not defined will use the package name of the UObject implementing this interface
	virtual NO_DISCARD FName GetModuleName() const { return NAME_None; }

}; // class IFlecsObjectRegistrationInterface
