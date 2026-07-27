// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "UnrealFlecsTests/Fixtures/FlecsTestReplicationBridge.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsTestReplicationBridge)

void UFlecsTestReplicationBridge::InitializeBridge()
{
	bInitialized = true;
}

void UFlecsTestReplicationBridge::DeinitializeBridge()
{
	bInitialized = false;
	Peer = nullptr;
}

void UFlecsTestReplicationBridge::PublishEntityLayout(
	const FFlecsReplicationLayoutDefinition& InLayoutDefinition)
{
	PublishedLayouts.Add(InLayoutDefinition);
	Super::PublishEntityLayout(InLayoutDefinition);

	if (Peer)
	{
		Peer->ReceiveEntityLayout(InLayoutDefinition);
	}
}

void UFlecsTestReplicationBridge::PublishNetEntity(
	const FFlecsNetRouteId& InRouteId,
	const FFlecsNetworkId& InNetworkId,
	const FFlecsEntityReplicationSnapshot& InSnapshot)
{
	PublishedRouteIds.Add(InRouteId);
	PublishedSnapshots.Emplace(InNetworkId, InSnapshot);

	if (Peer)
	{
		Peer->ReceiveNetEntity(InNetworkId, InSnapshot);
	}
}

void UFlecsTestReplicationBridge::SetPeer(UFlecsTestReplicationBridge* InPeer)
{
	Peer = InPeer;
}

void UFlecsTestReplicationBridge::ResetCapturedRecords()
{
	PublishedLayouts.Reset();
	PublishedRouteIds.Reset();
	PublishedSnapshots.Reset();
}
