// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsReplicationRouterBase.h"

#include "FlecsDefaultReplicationRouter.generated.h"

/** Routes every replicated entity to FFlecsNetRouteId::Default(). */
UCLASS()
class UNREALFLECS_API UFlecsDefaultReplicationRouter : public UFlecsReplicationRouterBase
{
	GENERATED_BODY()

public:
	virtual NO_DISCARD FFlecsNetRouteId RouteEntity(const FFlecsEntityHandle& InEntityHandle) const override;

}; // class UFlecsDefaultReplicationRouter
