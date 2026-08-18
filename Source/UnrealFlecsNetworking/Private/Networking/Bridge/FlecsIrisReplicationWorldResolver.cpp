// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Bridge/FlecsIrisReplicationWorldResolver.h"

#include "Engine/NetDriver.h"
#include "Net/Iris/ReplicationSystem/EngineReplicationBridge.h"

UWorld* UE::Flecs::Replication::GetReplicationBridgeWorld(const TSolidNotNull<const UObjectReplicationBridge*> InReplicationBridge)
{
	const UEngineReplicationBridge* const EngineReplicationBridge = CastChecked<UEngineReplicationBridge>(InReplicationBridge);
	const UNetDriver* const NetDriver = EngineReplicationBridge->GetNetDriver();
	return NetDriver ? NetDriver->GetWorld() : nullptr;
}
