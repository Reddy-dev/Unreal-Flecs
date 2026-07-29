// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsReplicationBridgeBase.h"

#include "FlecsIrisReplicationBridge.generated.h"

class UFlecsLayoutReplicator;
class UWorld;

/**
 * 
 */
UCLASS()
class UNREALFLECS_API UFlecsIrisReplicationBridge : public UFlecsReplicationBridgeBase
{
	GENERATED_BODY()

public:
	
	virtual void InitializeBridge() override;
	virtual void DeinitializeBridge() override;

	void BindLayoutReplicator(UFlecsLayoutReplicator* InLayoutReplicator);

	virtual void PublishEntityLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition) override;
	
	virtual void PublishNetEntity(
		const FFlecsNetRouteId& InRouteId,
		const FFlecsNetworkId& InNetworkId,
		const FFlecsEntityReplicationSnapshot& InSnapshot) override;
	
	virtual UFlecsNetShardBase* ResolveShard(const FFlecsNetRouteId& InRouteId, const FFlecsEntityHandle& InEntityHandle);
	
protected:
	void HandleWorldPreActorTick(UWorld* InWorld, ELevelTick InTickType, float InDeltaSeconds);

	UPROPERTY(Transient)
	TObjectPtr<UFlecsLayoutReplicator> LayoutReplicator;

	UPROPERTY()
	TMap<FFlecsNetRouteId, TObjectPtr<UFlecsNetShardBase>> ShardMap;

	FDelegateHandle WorldPreActorTickHandle;
	
}; // class UFlecsIrisReplicationBridge
