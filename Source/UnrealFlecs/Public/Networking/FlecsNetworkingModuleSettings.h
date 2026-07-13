// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/SoftObjectPtr.h"

#include "General/FlecsModuleSettings.h"

#include "FlecsNetworkingModuleSettings.generated.h"

/**
 * 
 */
UCLASS()
class UNREALFLECS_API UFlecsNetworkingModuleSettings : public UFlecsModuleSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config, Category = "Flecs | Networking")
	FName ReplicationProviderName = TEXT("Iris");

	UPROPERTY(EditAnywhere, Config, Category = "Flecs | Networking", meta = (ClampMin = "0.0"))
	float DefaultShardPollFrequency = 20.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Flecs | Networking", meta = (ClampMin = "0.0"))
	float DefaultShardStaticPriority = 1.0f;
	
}; // class UFlecsNetworkingModuleSettings
