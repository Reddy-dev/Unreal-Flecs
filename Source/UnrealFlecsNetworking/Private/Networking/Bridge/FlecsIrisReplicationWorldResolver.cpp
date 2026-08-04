// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Bridge/FlecsIrisReplicationWorldResolver.h"

#include "Engine/NetDriver.h"
#include "Net/Iris/ReplicationSystem/EngineReplicationBridge.h"

UWorld* UE::Flecs::Replication::GetReplicationBridgeWorld(
	const UObjectReplicationBridge* const InReplicationBridge)
{
	const UEngineReplicationBridge* const EngineReplicationBridge = Cast<UEngineReplicationBridge>(InReplicationBridge);
	const UNetDriver* const NetDriver = EngineReplicationBridge ? EngineReplicationBridge->GetNetDriver() : nullptr;
	return NetDriver ? NetDriver->GetWorld() : nullptr;
}
