// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"

#include "FlecsModuleSettings.generated.h"

/**
 * 
 */
UCLASS(Abstract, Config = Flecs, defaultconfig, AutoExpandCategories = "Flecs")
class UNREALFLECS_API UFlecsModuleSettings : public UObject
{
	GENERATED_BODY()

public:
	virtual void PostInitProperties() override;
	
}; // class UFlecsModuleSettings
