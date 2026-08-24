// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once


#include "Layout/FlecsReplicationLayoutId.h"

#include "FlecsReplicatedEntityComponent.generated.h"

/**
 * Per-entity opt-in marker for authoritative Flecs replication.
 *
 * Adding it on a server/listen server assigns an FFlecsNetworkId and starts
 * replication. Removing it, or destroying the entity, publishes a removal.
 * Descriptor-backed components and explicitly eligible descriptor-free
 * structural IDs are included in the entity's replicated layout. Only keys
 * with a component replication descriptor can carry payload bytes.
 */
USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsReplicatedEntityComponent
{
	GENERATED_BODY()
	
public:

	// @TODO:
	/*UPROPERTY()
	bool bInitialReplicationComplete = false;
	*/
	
	UPROPERTY()
	FFlecsReplicationLayoutId LayoutId;

	/** Registered Flecs replication profile name. None uses the inherited/default profile. */
	UPROPERTY()
	FName ProfileId = NAME_None;
	
}; // struct FFlecsReplicatedEntityComponent
