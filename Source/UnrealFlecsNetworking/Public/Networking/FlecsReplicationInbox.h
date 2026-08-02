// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"

#include "Properties/FlecsComponentProperties.h"

#include "Networking/FlecsNetworkId.h"
#include "Networking/Layout/FlecsReplicationSnapshot.h"

#include "FlecsReplicationInbox.generated.h"

USTRUCT()
struct UNREALFLECSNETWORKING_API FFlecsReplicationInboxUpdate
{
	GENERATED_BODY()

	UPROPERTY()
	FFlecsNetworkId NetworkId;

	UPROPERTY()
	FFlecsEntityReplicationSnapshot Snapshot;

	/** Snapshot revision, or the removal tombstone revision when bRemove is true. */
	UPROPERTY()
	uint32 StateRevision = 0;

	UPROPERTY()
	bool bRemove = false;

}; // struct FFlecsReplicationInboxUpdate

USTRUCT()
struct UNREALFLECSNETWORKING_API FFlecsReplicationInbox
{
	GENERATED_BODY()

	void EnqueueSnapshot(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot)
	{
		FFlecsReplicationInboxUpdate* ExistingUpdate = FindPendingUpdate(InNetworkId);
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

		FFlecsReplicationInboxUpdate& Update = Updates.Emplace_GetRef();
		Update.NetworkId = InNetworkId;
		Update.Snapshot = InSnapshot;
		Update.StateRevision = InSnapshot.StateRevision;
	}

	void EnqueueRemoval(const FFlecsNetworkId& InNetworkId, const uint32 InStateRevision)
	{
		FFlecsReplicationInboxUpdate* ExistingUpdate = FindPendingUpdate(InNetworkId);
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

		FFlecsReplicationInboxUpdate& Update = Updates.Emplace_GetRef();
		Update.NetworkId = InNetworkId;
		Update.StateRevision = InStateRevision;
		Update.bRemove = true;
	}

	NO_DISCARD TArray<FFlecsReplicationInboxUpdate> Drain()
	{
		return MoveTemp(Updates);
	}

private:

	NO_DISCARD FFlecsReplicationInboxUpdate* FindPendingUpdate(const FFlecsNetworkId& InNetworkId)
	{
		return Updates.FindByPredicate(
			[&InNetworkId](const FFlecsReplicationInboxUpdate& InUpdate)
			{
				return InUpdate.NetworkId == InNetworkId;
			});
	}

	UPROPERTY()
	TArray<FFlecsReplicationInboxUpdate> Updates;

}; // struct FFlecsReplicationInbox

template <>
struct TFlecsComponentTraits<FFlecsReplicationInbox> : public TFlecsComponentTraitsBase<FFlecsReplicationInbox>
{
	static constexpr bool Singleton = true;
}; // struct TFlecsComponentTraits<FFlecsReplicationInbox>
