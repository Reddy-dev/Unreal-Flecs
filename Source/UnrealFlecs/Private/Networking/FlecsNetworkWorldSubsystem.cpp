// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsNetworkWorldSubsystem.h"

#include "Engine/World.h"

#include "Networking/FlecsNetworkingModuleSettings.h"
#include "Networking/FlecsNetworkSubsystemSingleton.h"
#include "Networking/FlecsReplicationBridgeBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetworkWorldSubsystem)

UFlecsNetworkWorldSubsystem::UFlecsNetworkWorldSubsystem()
{
}

void UFlecsNetworkWorldSubsystem::OnFlecsWorldInitialized(const TSolidNotNull<UFlecsWorld*> InWorld)
{
	InWorld->Set<FFlecsNetworkSubsystemSingleton>(FFlecsNetworkSubsystemSingleton{ this });
	
	CreateReplicationBridge();
}

FFlecsNetworkId UFlecsNetworkWorldSubsystem::BeginReplicatingEntity(const FFlecsEntityHandle& EntityHandle)
{
	if UNLIKELY_IF(!IsNetModeServer())
	{
		UE_LOG(LogFlecsCore, Error, 
			TEXT("BeginReplicatingEntity called on a non-server world. This is not allowed."));
		return FFlecsNetworkId();
	}
	
	if UNLIKELY_IF(!EntityHandle.IsValid())
	{
		UE_LOG(LogFlecsCore, Error, 
			TEXT("BeginReplicatingEntity called with an invalid entity handle. This is not allowed."));
		return FFlecsNetworkId();
	}
	
	EntityHandle.Set<FFlecsNetworkId>(GenerateNewNetworkId());
	
	return FFlecsNetworkId();
}

void UFlecsNetworkWorldSubsystem::StopReplicatingEntity(const FFlecsEntityHandle& EntityHandle)
{
	if UNLIKELY_IF(!IsNetModeServer())
	{
		UE_LOG(LogFlecsCore, Error, 
			TEXT("StopReplicatingEntity called on a non-server world. This is not allowed."));
		return;
	}
	
	if UNLIKELY_IF(!EntityHandle.IsValid())
	{
		UE_LOG(LogFlecsCore, Error, 
			TEXT("StopReplicatingEntity called with an invalid entity handle. This is not allowed."));
		return;
	}
	
	if UNLIKELY_IF(!EntityHandle.Has<FFlecsNetworkId>())
	{
		UE_LOG(LogFlecsCore, Error, 
			TEXT("StopReplicatingEntity called on an entity that is not being replicated. This is not allowed."));
		return;
	}
	
	
}

void UFlecsNetworkWorldSubsystem::StopReplicatingEntity(const FFlecsNetworkId& NetworkId)
{
	
}

bool UFlecsNetworkWorldSubsystem::IsNetModeServer() const
{
	return GetWorld()->GetNetMode() == NM_DedicatedServer || GetWorld()->GetNetMode() == NM_ListenServer;
}

void UFlecsNetworkWorldSubsystem::CreateReplicationBridge()
{
	const TSoftClassPtr<AFlecsReplicationBridgeBase> ReplicationBridgeClass = GetNetworkingModuleSettings()->ReplicationBridgeInfoClass;
	const TSubclassOf<AFlecsReplicationBridgeBase> ReplicationBridgeSubclass = ReplicationBridgeClass.LoadSynchronous();
	
	if UNLIKELY_IF(!ReplicationBridgeSubclass)
	{
		UE_LOG(LogFlecsCore, Error, 
			TEXT("ReplicationBridgeInfoClass is not set in the FlecsNetworkingModuleSettings. Please set it to a valid subclass of AFlecsReplicationBridgeBase."));
		return;
	}
	
	FlecsReplicationBridge = GetWorld()->SpawnActor<AFlecsReplicationBridgeBase>(ReplicationBridgeSubclass);
}

TSolidNotNull<UFlecsNetworkingModuleSettings*> UFlecsNetworkWorldSubsystem::GetNetworkingModuleSettings() const
{
	return GetDefault<UFlecsNetworkingModuleSettings>();
}

FFlecsNetworkId UFlecsNetworkWorldSubsystem::GenerateNewNetworkId()
{
	return FFlecsNetworkId(++NextNetworkId);
}
