// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "flecs.h"


#include "Types/SolidNotNull.h"

#include "FlecsReplicationLayoutDefinition.h"

#include "FlecsReplicationLayoutId.h"

struct FFlecsEntityHandle;
class UFlecsWorld;

struct FFlecsReplicationKey;

/**
 * Per-world cache of locally generated and remotely validated layouts.
 *
 * Local layouts are cached by Flecs table because all entities in a table have
 * the same replicated structure. Remote definitions are checked against their
 * deterministic ID before being retained.
 */
class UNREALFLECSNETWORKING_API FFlecsReplicationLayoutRegistry
{
public:
	/** Computes the deterministic layout ID from a sorted key list. */
	static NO_DISCARD FFlecsReplicationLayoutId ComputeLayoutId(const TArray<FFlecsReplicationKey>& Keys);
	
	/** Builds or reuses a local layout for Entity's current Flecs table. */
	TValueOrError<const FFlecsReplicationLayoutDefinition*, FString> BuildForEntity(
		const TSolidNotNull<const UFlecsWorldInterfaceObject*> World,
		const FFlecsEntityHandle& Entity,
		OUT bool& bOutCreatedNewLayout);
	
	NO_DISCARD bool HasDefinition(FFlecsReplicationLayoutId Id) const;
	
	/** Finds a previously generated or accepted layout definition. */
	NO_DISCARD const FFlecsReplicationLayoutDefinition* Find(FFlecsReplicationLayoutId Id) const;
	
	/** Adds an already validated remote layout, rejecting identity collisions. */
	TValueOrError<bool, FString> AddRemoteDefinition(const FFlecsReplicationLayoutDefinition& Definition,
		const UFlecsWorldInterfaceObject* World);
	
	NO_DISCARD bool HasPendingLayouts() const;
	
	void TryConsumePendingLayouts(const TSolidNotNull<const UFlecsWorldInterfaceObject*> World);

private:
	// @TODO: Handle layout invalidation when Flecs deletes a table.
	// Layouts are currently retained even if Flecs deletes the originating table.
	
	TMap<const flecs::table_t*, FFlecsReplicationLayoutId> TableCache;
	TMap<FFlecsReplicationLayoutId, FFlecsReplicationLayoutDefinition> Definitions;
	
	TArray<FFlecsReplicationLayoutDefinition> PendingLayouts;
	
	void AddPendingLayout(const FFlecsReplicationLayoutDefinition& Definition);
	
	NO_DISCARD bool ValidateLayoutDefinition(const FFlecsReplicationLayoutDefinition& Definition,
		const TSolidNotNull<const UFlecsWorldInterfaceObject*> World) const;
	
	
}; // class FFlecsReplicationLayoutRegistry
