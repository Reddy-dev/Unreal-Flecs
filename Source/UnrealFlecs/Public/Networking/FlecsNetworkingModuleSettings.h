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

	/** Poll frequency inherited by Iris pages whose route does not provide one. */
	UPROPERTY(EditAnywhere, Config, Category = "Flecs | Networking", meta = (ClampMin = "0.0"))
	float DefaultShardPollFrequency = 20.0f;

	/** Static priority inherited by Iris pages whose route does not provide one. */
	UPROPERTY(EditAnywhere, Config, Category = "Flecs | Networking", meta = (ClampMin = "0.0"))
	float DefaultShardStaticPriority = 1.0f;

	/** Entity capacity inherited by routes whose PageEntityLimit is zero. */
	UPROPERTY(EditAnywhere, Config, Category = "Flecs | Networking", meta = (ClampMin = "1"))
	uint16 DefaultPageEntityLimit = 256;

	/** Retained serialized entity payload capacity inherited by routes whose byte limit is zero. */
	UPROPERTY(EditAnywhere, Config, Category = "Flecs | Networking", meta = (ClampMin = "1"))
	uint32 DefaultPageRetainedPayloadByteLimit = 256 * 1024;
	
}; // class UFlecsNetworkingModuleSettings
