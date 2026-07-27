// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsIrisReplicationBridge.h"

#include "Networking/Shards/FlecsNetShardBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsIrisReplicationBridge)

void UFlecsIrisReplicationBridge::InitializeBridge()
{
}

void UFlecsIrisReplicationBridge::DeinitializeBridge()
{
	
}

void UFlecsIrisReplicationBridge::PublishEntityLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition)
{
	
}

void UFlecsIrisReplicationBridge::PublishNetEntity(
	const FFlecsNetRouteId& InRouteId,
	const FFlecsNetworkId& InNetworkId,
	const FFlecsEntityReplicationSnapshot& InSnapshot)
{
	const TSolidNotNull<UFlecsNetShardBase*> Shard = ResolveShard(InRouteId);
	Shard->
}
