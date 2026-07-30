// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Networking/Bridge/FlecsReplicationBridgeBase.h"

#include "FlecsTestReplicationBridge.generated.h"

UCLASS()
class UNREALFLECSTESTS_API UFlecsTestReplicationBridge : public UFlecsReplicationBridgeBase
{
	GENERATED_BODY()

public:
	using UFlecsReplicationBridgeBase::PublishNetEntity;

	virtual void InitializeBridge() override;
	virtual void DeinitializeBridge() override;

	virtual void PublishEntityLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition) override;
	virtual void PublishNetEntity(
		const FFlecsNetRouteId& InRouteId,
		const FFlecsNetworkId& InNetworkId,
		const FFlecsEntityReplicationSnapshot& InSnapshot) override;

	void SetPeer(UFlecsTestReplicationBridge* InPeer);
	void ResetCapturedRecords();

	NO_DISCARD bool IsInitialized() const
	{
		return bInitialized;
	}

	NO_DISCARD const TArray<FFlecsReplicationLayoutDefinition>& GetPublishedLayouts() const
	{
		return PublishedLayouts;
	}

	NO_DISCARD const TArray<TPair<FFlecsNetworkId, FFlecsEntityReplicationSnapshot>>& GetPublishedSnapshots() const
	{
		return PublishedSnapshots;
	}

	NO_DISCARD const TArray<FFlecsNetRouteId>& GetPublishedRouteIds() const
	{
		return PublishedRouteIds;
	}

private:
	UPROPERTY(Transient)
	TObjectPtr<UFlecsTestReplicationBridge> Peer = nullptr;

	UPROPERTY(Transient)
	TArray<FFlecsReplicationLayoutDefinition> PublishedLayouts;

	TArray<TPair<FFlecsNetworkId, FFlecsEntityReplicationSnapshot>> PublishedSnapshots;

	TArray<FFlecsNetRouteId> PublishedRouteIds;

	bool bInitialized = false;
}; // class UFlecsTestReplicationBridge
