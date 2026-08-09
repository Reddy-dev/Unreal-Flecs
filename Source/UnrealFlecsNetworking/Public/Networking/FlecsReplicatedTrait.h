// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsReplicatedTrait.generated.h"

/**
 * Marker for Flecs IDs whose structure participates in replication.
 *
 * Component types registered with Replicate enabled receive this automatically.
 * It may also be added explicitly to a stable-symbol or stable-path ID entity
 * that should replicate structurally without a payload descriptor. Networked ID
 * entities are structurally eligible through FFlecsNetworkId instead. This is
 * not the per-entity replication opt-in; use FFlecsReplicatedEntityComponent to
 * mark an entity whose state should be gathered and published.
 */
USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsReplicatedTrait
{
	GENERATED_BODY()
}; // struct FFlecsReplicatedTrait

template <>
struct TFlecsComponentTraits<FFlecsReplicatedTrait> : public TFlecsComponentTraitsBase<FFlecsReplicatedTrait>
{
	static constexpr bool Trait = true;
}; // struct TFlecsComponentTraits<FFlecsReplicatedTrait>
