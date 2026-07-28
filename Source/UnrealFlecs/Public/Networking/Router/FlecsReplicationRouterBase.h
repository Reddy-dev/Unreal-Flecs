// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"

#include "SolidMacros/Macros.h"

#include "FlecsNetRouteId.h"

#include "FlecsReplicationRouterBase.generated.h"

struct FFlecsEntityHandle;

UENUM()
enum class EFlecsReplicationRoutedShardType
{
	Proxy,
	Paged
}; // enum class EFlecsReplicationRoutedShardType

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
	
	// @TODO: Refactor as this might not necessarily always be included in another persons impl
	virtual NO_DISCARD EFlecsReplicationRoutedShardType GetRoutedShardType(
		const FFlecsEntityHandle& InEntityHandle, const FFlecsNetRouteId& InRouteId) const;

}; // class UFlecsReplicationRouterBase
