// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/SoftObjectPtr.h"

#include "General/FlecsModuleSettings.h"

#include "FlecsNetworkingModuleSettings.generated.h"

class AFlecsReplicationBridgeBase;

/**
 * 
 */
UCLASS()
class UNREALFLECS_API UFlecsNetworkingModuleSettings : public UFlecsModuleSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config, Category = "Flecs | Networking")
	TSoftClassPtr<AFlecsReplicationBridgeBase> ReplicationBridgeInfoClass;
	
}; // class UFlecsNetworkingModuleSettings
