// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsIrisReplicationBridge.h"

#include "Networking/FlecsNetworkWorldSubsystem.h"
#include "Networking/Layout/FlecsLayoutReplicator.h"
#include "Networking/Router/FlecsReplicationRouterBase.h"
#include "Networking/Shards/FlecsNetShardBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsIrisReplicationBridge)

void UFlecsIrisReplicationBridge::InitializeBridge()
{
	if (!HasAuthority())
	{
		return;
	}

	LayoutReplicator = NewObject<UFlecsLayoutReplicator>(this);
	check(IsValid(LayoutReplicator));
	LayoutReplicator->InitializeReplicator(this);

	if (!LayoutReplicator->TryStartReplication())
	{
		WorldPreActorTickHandle = FWorldDelegates::OnWorldPreActorTick.AddUObject(
			this, &UFlecsIrisReplicationBridge::HandleWorldPreActorTick);
	}
}

void UFlecsIrisReplicationBridge::DeinitializeBridge()
{
	if (WorldPreActorTickHandle.IsValid())
	{
		FWorldDelegates::OnWorldPreActorTick.Remove(WorldPreActorTickHandle);
		WorldPreActorTickHandle.Reset();
	}

	if (LayoutReplicator)
	{
		LayoutReplicator->DeinitializeReplicator();
		LayoutReplicator = nullptr;
	}

}

void UFlecsIrisReplicationBridge::BindLayoutReplicator(UFlecsLayoutReplicator* InLayoutReplicator)
{
	check(IsValid(InLayoutReplicator));
	check(!HasAuthority());

	LayoutReplicator = InLayoutReplicator;
	LayoutReplicator->BindReplicationBridge(this);
}

void UFlecsIrisReplicationBridge::HandleWorldPreActorTick(
	UWorld* InWorld,
	ELevelTick,
	float)
{
	if (InWorld != GetWorld() || !LayoutReplicator)
	{
		return;
	}

	if (LayoutReplicator->TryStartReplication())
	{
		FWorldDelegates::OnWorldPreActorTick.Remove(WorldPreActorTickHandle);
		WorldPreActorTickHandle.Reset();
	}
}

void UFlecsIrisReplicationBridge::PublishEntityLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition)
{
	check(IsValid(LayoutReplicator));
	LayoutReplicator->PublishLayout(InLayoutDefinition);
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
