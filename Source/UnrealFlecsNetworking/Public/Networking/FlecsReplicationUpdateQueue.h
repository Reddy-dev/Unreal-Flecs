// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Networking/FlecsNetworkId.h"
#include "Networking/Layout/FlecsReplicationSnapshot.h"

/** One deferred client-side replication update owned by the world subsystem. */
struct FFlecsReplicationQueuedUpdate
{
	FFlecsNetworkId NetworkId;
	FFlecsEntityReplicationSnapshot Snapshot;
	uint32 StateRevision = 0;
	bool bRemove = false;

}; // struct FFlecsReplicationQueuedUpdate

/** Coalesces deferred snapshots and removals by network ID. */
class FFlecsReplicationUpdateQueue
{
public:
	void EnqueueSnapshot(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot)
	{
		FFlecsReplicationQueuedUpdate* ExistingUpdate = FindPendingUpdate(InNetworkId);
		
		if (ExistingUpdate)
		{
			if (ExistingUpdate->StateRevision > InSnapshot.StateRevision)
			{
				return;
			}

			ExistingUpdate->Snapshot = InSnapshot;
			ExistingUpdate->StateRevision = InSnapshot.StateRevision;
			ExistingUpdate->bRemove = false;
			return;
		}

		FFlecsReplicationQueuedUpdate& Update = Updates.Emplace_GetRef();
		Update.NetworkId = InNetworkId;
		Update.Snapshot = InSnapshot;
		Update.StateRevision = InSnapshot.StateRevision;
		Update.bRemove = false;
	}

	void EnqueueRemoval(const FFlecsNetworkId& InNetworkId, const uint32 InStateRevision)
	{
		FFlecsReplicationQueuedUpdate* ExistingUpdate = FindPendingUpdate(InNetworkId);
		if (ExistingUpdate)
		{
			if (ExistingUpdate->StateRevision > InStateRevision)
			{
				return;
			}

			ExistingUpdate->Snapshot = {};
			ExistingUpdate->StateRevision = InStateRevision;
			ExistingUpdate->bRemove = true;
			return;
		}

		FFlecsReplicationQueuedUpdate& Update = Updates.Emplace_GetRef();
		Update.NetworkId = InNetworkId;
		Update.StateRevision = InStateRevision;
		Update.bRemove = true;
	}

	NO_DISCARD TArray<FFlecsReplicationQueuedUpdate> Drain()
	{
		return MoveTemp(Updates);
	}

	NO_DISCARD int32 Num() const
	{
		return Updates.Num();
	}

	void Reset()
	{
		Updates.Reset();
	}

private:
	NO_DISCARD FFlecsReplicationQueuedUpdate* FindPendingUpdate(const FFlecsNetworkId& InNetworkId)
	{
		return Updates.FindByPredicate(
			[&InNetworkId](const FFlecsReplicationQueuedUpdate& InUpdate)
			{
				return InUpdate.NetworkId == InNetworkId;
			});
	}

	TArray<FFlecsReplicationQueuedUpdate> Updates;

}; // class FFlecsReplicationUpdateQueue
