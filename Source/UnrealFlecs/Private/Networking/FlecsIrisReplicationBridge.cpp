// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsIrisReplicationBridge.h"

#include "Networking/FlecsNetworkWorldSubsystem.h"
#include "Networking/Router/FlecsReplicationRouterBase.h"
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
	
	
}

UFlecsNetShardBase* UFlecsIrisReplicationBridge::ResolveShard(const FFlecsNetRouteId& InRouteId,
	const FFlecsEntityHandle& InEntityHandle)
{
	if UNLIKELY_IF(!InRouteId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid route id passed to ResolveShard"));
		return nullptr;
	}
	
	EFlecsReplicationRoutedShardType ShardType = GetNetworkWorldSubsystem()->GetReplicationRouter()
		->GetRoutedShardType(InEntityHandle, InRouteId);
	
	// @TODO: switch to a switch
	if (ShardType == EFlecsReplicationRoutedShardType::Proxy)
	{
		
	}
	else if (ShardType == EFlecsReplicationRoutedShardType::Paged)
	{
		
	}
	
	checkf(false, TEXT("Unsupported shard type %d"), static_cast<int32>(ShardType));
	return nullptr;
}