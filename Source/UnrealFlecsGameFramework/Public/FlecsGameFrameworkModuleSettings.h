// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "General/FlecsModuleSettings.h"

#include "FlecsGameFrameworkModuleSettings.generated.h"

/**
 * 
 */
UCLASS()
class UNREALFLECSGAMEFRAMEWORK_API UFlecsGameFrameworkModuleSettings : public UFlecsModuleSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Viewers", config)
	uint8 bUsePlayerPawnLocationInsteadOfCamera : 1 = false;
	
	/** If true, all PlayerControllers will be gathered as viewers for LOD calculations. */
	UPROPERTY(EditDefaultsOnly, Category = "Viewers", config)
	uint8 bGatherPlayerControllers : 1 = true;

	/** If true, all streaming sources will be gathered as viewers for LOD calculations. */
	UPROPERTY(EditDefaultsOnly, Category = "Viewers", config)
	uint8 bGatherStreamingSources : 1 = true;

	/** Whether using non-player actors as LOD Viewers is supported. */
	UPROPERTY(EditDefaultsOnly, Category = "Viewers", config)
	uint8 bAllowNonPlayerViewerActors : 1 = true;
	
}; // class UFlecsGameFrameworkModuleSettings
