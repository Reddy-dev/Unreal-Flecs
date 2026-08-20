// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsModuleSettings.h"

#include "FlecsEntitySettings.generated.h"

/**
 * 
 */
UCLASS(Config = Flecs, DefaultConfig, DisplayName = "Flecs Entity Settings")
class UNREALFLECS_API UFlecsEntitySettings : public UFlecsModuleSettings
{
	GENERATED_BODY()

public:
	UFlecsEntitySettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
}; // class UFlecsEntitySettings
