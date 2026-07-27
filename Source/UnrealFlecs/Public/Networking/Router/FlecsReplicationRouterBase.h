// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"

#include "Networking/Shards/FlecsNetRouteId.h"

#include "FlecsReplicationRouterBase.generated.h"

struct FFlecsEntityHandle;

/**
 * Selects the logical route used to publish a replicated Flecs entity.
 *
 * Routers own route-selection policy only. Shard storage and transport remain
 * owned by the replication bridge.
 */
UCLASS(Abstract, BlueprintType, NotBlueprintable)
class UNREALFLECS_API UFlecsReplicationRouterBase : public UObject
{
	GENERATED_BODY()

public:
	virtual void InitializeRouter() {}
	virtual void DeinitializeRouter() {}

	virtual NO_DISCARD FFlecsNetRouteId RouteEntity(const FFlecsEntityHandle& InEntityHandle) const
		PURE_VIRTUAL(UFlecsReplicationRouterBase::RouteEntity, return FFlecsNetRouteId(););

}; // class UFlecsReplicationRouterBase
