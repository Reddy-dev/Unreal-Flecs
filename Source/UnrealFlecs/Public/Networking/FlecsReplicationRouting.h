// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "Networking/FlecsReplicationTypes.h"

#include "FlecsReplicationRouting.generated.h"

/** Optional, non-replicated per-entity override consumed by the default router. */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicationRouting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	FFlecsReplicationRouteDescriptor Route;
}; // struct FFlecsReplicationRouting

template <>
struct TFlecsComponentTraits<FFlecsReplicationRouting> : public TFlecsComponentTraitsBase<FFlecsReplicationRouting>
{
	//static constexpr bool DontFragment = true;
}; // struct TFlecsComponentTraits<FFlecsReplicationRouting>
