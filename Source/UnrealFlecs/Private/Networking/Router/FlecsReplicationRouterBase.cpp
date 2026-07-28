// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Router/FlecsReplicationRouterBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationRouterBase)

EFlecsReplicationRoutedShardType UFlecsReplicationRouterBase::GetRoutedShardType(
	const FFlecsEntityHandle& InEntityHandle, const FFlecsNetRouteId& InRouteId) const
{
	return EFlecsReplicationRoutedShardType::Proxy;
}
