// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

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
class UNREALFLECS_API FFlecsReplicationLayoutRegistry
{
public:
	/** Computes the deterministic layout ID from a sorted key list. */
	static NO_DISCARD FFlecsReplicationLayoutId ComputeLayoutId(const TArray<FFlecsReplicationKey>& Keys);
	
	// @TODO: ReturnOrError
	/** Builds or reuses a local layout for Entity's current Flecs table. */
	const FFlecsReplicationLayoutDefinition* BuildForEntity(const TSolidNotNull<const UFlecsWorld*> World,
		const FFlecsEntityHandle& Entity, OUT bool& bOutWasCreated, OUT FString& OutError);
	
	/** Finds a previously generated or accepted layout definition. */
	NO_DISCARD const FFlecsReplicationLayoutDefinition* Find(FFlecsReplicationLayoutId Id) const;
	
	/** Adds an already validated remote layout, rejecting identity collisions. */
	bool AddRemoteDefinition(const FFlecsReplicationLayoutDefinition& Definition, OUT FString& OutError);

private:
	// @TODO: Handle Table destruction and remove from cache.
	TMap<const flecs::table_t*, FFlecsReplicationLayoutId> TableCache;
	TMap<FFlecsReplicationLayoutId, FFlecsReplicationLayoutDefinition> Definitions;
	
}; // class FFlecsReplicationLayoutRegistry