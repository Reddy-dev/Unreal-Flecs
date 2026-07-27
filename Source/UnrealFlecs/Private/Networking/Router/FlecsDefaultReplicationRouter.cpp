// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Router/FlecsDefaultReplicationRouter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsDefaultReplicationRouter)

FFlecsNetRouteId UFlecsDefaultReplicationRouter::RouteEntity(const FFlecsEntityHandle&) const
{
	return FFlecsNetRouteId::Default();
}
