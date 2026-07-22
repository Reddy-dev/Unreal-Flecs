// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "SolidMacros/Macros.h"

#include "FlecsNetworkId.h"

#include "FlecsNetworkIDGeneratorInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(NotBlueprintable, meta = (CannotImplementInterfaceInBlueprint))
class UFlecsNetworkIDGeneratorInterface : public UInterface
{
	GENERATED_BODY()
}; // class UFlecsNetworkIDGeneratorInterface

/**
 * 
 */
class UNREALFLECS_API IFlecsNetworkIDGeneratorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual NO_DISCARD FFlecsNetworkId GenerateNetworkId() = 0;
	virtual bool ReleaseNetworkId(const FFlecsNetworkId& NetworkId) = 0;
	virtual void ResetNetworkIdGenerator() = 0;

}; // class IFlecsNetworkIDGeneratorInterface
