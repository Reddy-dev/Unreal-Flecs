// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsReplicatedEntityComponent.generated.h"

/**
 * Per-entity opt-in marker for authoritative Flecs replication.
 *
 * Adding it on a server/listen server assigns an FFlecsNetworkId and starts
 * replication. Removing it, or destroying the entity, publishes a removal.
 * Only component types registered with Replicate enabled are included in the
 * entity's replicated layout and payload.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicatedEntityComponent
{
	GENERATED_BODY()
}; // struct FFlecsReplicatedEntityComponent
