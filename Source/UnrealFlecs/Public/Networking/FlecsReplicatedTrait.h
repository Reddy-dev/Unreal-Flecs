// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsReplicatedTrait.generated.h"

/**
 * Internal marker placed on component types registered with Replicate enabled.
 *
 * This identifies a replicated component definition to Flecs observers; it is
 * not the per-entity replication opt-in. Use FFlecsReplicatedEntityComponent
 * to mark an entity for replication.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicatedTrait
{
	GENERATED_BODY()
}; // struct FFlecsReplicatedTrait
