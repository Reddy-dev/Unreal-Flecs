// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/SoftObjectPtr.h"

#include "General/FlecsModuleSettings.h"

#include "FlecsNetworkingModuleSettings.generated.h"

/** Runtime configuration for the Flecs replication core and its selected transport provider. */
UCLASS()
class UNREALFLECS_API UFlecsNetworkingModuleSettings : public UFlecsModuleSettings
{
	GENERATED_BODY()

public:
	/** Name registered through FFlecsReplicationTransportRegistry; Iris is the built-in provider. */
	UPROPERTY(EditAnywhere, Config, Category = "Flecs | Networking")
	FName ReplicationProviderName = TEXT("Iris");

	/** Poll frequency assigned to each Iris aggregate shard created by the default adapter. */
	UPROPERTY(EditAnywhere, Config, Category = "Flecs | Networking", meta = (ClampMin = "0.0"))
	float DefaultShardPollFrequency = 20.0f;

	/** Static priority assigned to each Iris aggregate shard created by the default adapter. */
	UPROPERTY(EditAnywhere, Config, Category = "Flecs | Networking", meta = (ClampMin = "0.0"))
	float DefaultShardStaticPriority = 1.0f;
	
}; // class UFlecsNetworkingModuleSettings
