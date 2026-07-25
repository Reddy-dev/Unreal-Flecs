// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsReplicationBridgeBase.h"

#include "FlecsIrisReplicationBridge.generated.h"

/**
 * 
 */
UCLASS()
class UNREALFLECS_API UFlecsIrisReplicationBridge : public UFlecsReplicationBridgeBase
{
	GENERATED_BODY()

public:
	virtual void PublishEntityLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition) override;
	
	virtual void PublishNetEntity(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot) override;
	
protected:
	
}; // class UFlecsIrisReplicationBridge
