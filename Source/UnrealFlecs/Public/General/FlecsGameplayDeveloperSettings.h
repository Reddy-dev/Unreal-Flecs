// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/DeveloperSettings.h"

#include "Types/SolidNotNull.h"

#include "FlecsGameplayDeveloperSettings.generated.h"

class UFlecsModuleSettings;

/**
 * 
 */
UCLASS(BlueprintType, Config = Flecs, DefaultConfig, DisplayName = "Flecs Gameplay Settings", AutoExpandCategories = "Flecs")
class UNREALFLECS_API UFlecsGameplayDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "Flecs", NoClear, EditFixedSize, meta = (EditInline))
	TMap<FName, TObjectPtr<UFlecsModuleSettings>> ModuleSettings;
	
	void RegisterModuleSettings(const TSolidNotNull<UFlecsModuleSettings*> InModuleSettings);
	
}; // class UFlecsGameplayDeveloperSettings
