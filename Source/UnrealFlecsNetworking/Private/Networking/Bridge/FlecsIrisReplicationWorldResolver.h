// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

class UObjectReplicationBridge;
class UWorld;

namespace UE::Flecs::Replication
{

	/**
	 * Iris creates UEngineReplicationBridge with the transient package as its outer,
	 * so resolve its world through its NetDriver instead of UObject::GetWorld().
	 */
	NO_DISCARD UWorld* GetReplicationBridgeWorld(const TSolidNotNull<const UObjectReplicationBridge*> InReplicationBridge);

} // namespace UE::Flecs::Replication
