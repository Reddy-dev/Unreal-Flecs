// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Systems/FlecsNetDirtySystem.h"

#include "Networking/FlecsNetDirtyTag.h"
#include "Networking/FlecsNetworkSubsystemSingleton.h"
#include "Networking/FlecsNetworkWorldSubsystem.h"
#include "Networking/FlecsReplicatedEntityComponent.h"
#include "Networking/FlecsReplicationBridgeBase.h"
#include "Networking/Layout/FlecsReplicationSnapshot.h"
#include "Networking/Router/FlecsReplicationRouterBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetDirtySystem)

UFlecsNetDirtySystem::UFlecsNetDirtySystem()
{
}

void UFlecsNetDirtySystem::BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld,
                                       TFlecsSystemBuilder<>& InBuilder) const
{
	InBuilder
		.With<FFlecsNetDirtyTag>().ReadWrite() // 0
		.With<FFlecsReplicatedEntityComponent&>() // 1
		.With<const FFlecsNetworkId>() // 2
		.With<const FFlecsNetworkSubsystemSingleton>(); // 3
}

void UFlecsNetDirtySystem::EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
                                        flecs::iter& InIterator, const FFlecsId InIndex)
{
	FFlecsReplicatedEntityComponent& ReplicatedComponent = InIterator.field_at<FFlecsReplicatedEntityComponent>(1, InIndex);
	const FFlecsNetworkId& NetworkId = InIterator.field_at<const FFlecsNetworkId>(2, InIndex);
	
	const TSolidNotNull<UFlecsNetworkWorldSubsystem*> NetworkSubsystem 
		= InIterator.field_at<const FFlecsNetworkSubsystemSingleton>(3, 0).GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
		
	const FFlecsEntityHandle EntityHandle = InIterator.entity(InIndex);

	const FFlecsNetRouteId RouteId = NetworkSubsystem->GetReplicationRouter()->RouteEntity(EntityHandle);
	if UNLIKELY_IF(!RouteId.IsValid())
	{
		NetworkSubsystem->GetReplicationBridge()->HandleProtocolError(FString::Printf(
			TEXT("Replication router returned an invalid route for entity %s"),
			*EntityHandle.ToString()));

		EntityHandle.Remove<FFlecsNetDirtyTag>();
		return;
	}
	
	bool bCreatedNewLayout = false;
		
	TValueOrError<const FFlecsReplicationLayoutDefinition*, FString> LayoutResult = 
		NetworkSubsystem->GetLayoutRegistry().BuildForEntity(InWorld, EntityHandle, bCreatedNewLayout);
		
	// @TODO: Remove this in shipping?
	if UNLIKELY_IF(LayoutResult.HasError())
	{
		NetworkSubsystem->GetReplicationBridge()->HandleProtocolError(FString::Printf(
			TEXT("Failed to build replication layout for entity %s: %s"),
			*EntityHandle.ToString(), *LayoutResult.GetError()));
		
		EntityHandle.Remove<FFlecsNetDirtyTag>();
		return;
	}
		
	// @TODO: DontFragment
	
	const TSolidNotNull<const FFlecsReplicationLayoutDefinition*> LayoutDefinition = LayoutResult.GetValue();
		
	const FFlecsReplicationLayoutId NewLayoutId = LayoutResult.GetValue()->LayoutId;
		
	ReplicatedComponent.LayoutId = NewLayoutId;
		
	FFlecsEntityReplicationSnapshot& Snapshot = NetworkSubsystem->GetReplicationSnapshots().FindOrAdd(NetworkId);
	Snapshot.LayoutId = NewLayoutId;
	Snapshot.FillFromEntity(EntityHandle, NetworkSubsystem->GetLayoutRegistry());
	
	if (bCreatedNewLayout)
	{
		NetworkSubsystem->GetReplicationBridge()->PublishEntityLayout(*LayoutDefinition);
	}
		
	NetworkSubsystem->GetReplicationBridge()->PublishNetEntity(RouteId, NetworkId, Snapshot);

	EntityHandle.Remove<FFlecsNetDirtyTag>();
}

EFlecsObjectRegistrationNetworkFlags UFlecsNetDirtySystem::GetObjectRegistrationNetworkFlags() const
{
	return EFlecsObjectRegistrationNetworkFlags::Server;
}
