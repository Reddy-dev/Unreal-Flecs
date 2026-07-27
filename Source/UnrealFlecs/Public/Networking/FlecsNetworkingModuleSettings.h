// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Templates/SubclassOf.h"

#include "General/FlecsModuleSettings.h"

#include "FlecsNetworkingModuleSettings.generated.h"

class UFlecsReplicationBridgeBase;
class UFlecsReplicationRouterBase;

/** Runtime configuration for the Flecs replication core and its selected transport provider. */
UCLASS()
class UNREALFLECS_API UFlecsNetworkingModuleSettings : public UFlecsModuleSettings
{
	GENERATED_BODY()

public:
	virtual void PostInitProperties() override;
	
	UPROPERTY(EditAnywhere, Config,
		meta = (AllowAbstract = false, MustImplement = "/Script/UnrealFlecs.FlecsNetworkIdGeneratorInterface"))
	TSubclassOf<UObject> NetworkIdGeneratorClass;
	
	UPROPERTY(EditAnywhere, Config, meta = (AllowAbstract = false))
	TSubclassOf<UFlecsReplicationBridgeBase> ReplicationBridgeClass;

	UPROPERTY(EditAnywhere, Config, meta = (AllowAbstract = false))
	TSubclassOf<UFlecsReplicationRouterBase> ReplicationRouterClass;
	
}; // class UFlecsNetworkingModuleSettings
