// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "SolidMacros/Macros.h"

#include "FlecsWorldSettingsInterface.generated.h"

class UFlecsWorldSettingsAsset;

UINTERFACE()
class UFlecsWorldSettingsInterface : public UInterface
{
	GENERATED_BODY()
}; // class UFlecsWorldSettingsInterface

/**
 * @brief
 */
class UNREALFLECS_API IFlecsWorldSettingsInterface
{
	GENERATED_BODY()

public:
	NO_DISCARD virtual bool EnableFlecsOnWorld() const = 0;
	virtual UFlecsWorldSettingsAsset* GetFlecsWorldSettingsAsset() const = 0;

}; // class IFlecsWorldSettingsInterface
