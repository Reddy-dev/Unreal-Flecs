// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Templates/SubclassOf.h"

#include "General/FlecsModuleSettings.h"

#include "FlecsNetworkingModuleSettings.generated.h"

class UFlecsReplicationBridgeBase;

/** Runtime configuration for the Flecs replication core and its selected transport provider. */
UCLASS()
class UNREALFLECS_API UFlecsNetworkingModuleSettings : public UFlecsModuleSettings
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, Config,
		meta = (AllowAbstract = false, MustImplement = "/Script/UnrealFlecs.FlecsNetworkIdGeneratorInterface"))
	TSubclassOf<UObject> NetworkIdGeneratorClass;
	
	UPROPERTY(EditAnywhere, Config, meta = (AllowAbstract = false))
	TSubclassOf<UFlecsReplicationBridgeBase> ReplicationBridgeClass;
	
}; // class UFlecsNetworkingModuleSettings
