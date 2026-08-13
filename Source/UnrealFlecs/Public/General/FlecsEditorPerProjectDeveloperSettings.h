// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/DeveloperSettings.h"

#include "FlecsExplorerURLSettings.h"

#include "FlecsEditorPerProjectDeveloperSettings.generated.h"

UCLASS(MinimalAPI, Config=EditorPerProjectUserSettings, DisplayName="Flecs Per-Project Editor Settings")
class UFlecsEditorPerProjectDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Config, Category = "Explorer")
	bool bOpenFlecsExplorerOnPlay = true;

	UPROPERTY(EditAnywhere, Config, Category = "Explorer")
	bool bOpenFlecsExplorerExternally = false;

	UPROPERTY(EditAnywhere, Config, Category = "Explorer", meta = (ClampMin = "1", UIMin = "1", UIMax = "240"))
	uint32 InEditorExplorerFrameRate = 60;
	
	/**
	 * @brief Override URL used to connect to the Flecs Explorer for this project. Leave empty to use the global editor setting.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Explorer")
	TOptional<FFlecsEditorExplorerURL> FlecsExplorerURLOverride;

}; // class UFlecsEditorPerProjectDeveloperSettings
