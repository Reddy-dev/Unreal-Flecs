// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Worlds/FlecsAbstractWorldSubsystem.h"
#include "Networking/FlecsReplicationTransportBase.h"

#include "FlecsNetworkWorldSubsystem.generated.h"

class UFlecsNetworkingModuleSettings;

/**
 * Per-UWorld coordinator for Flecs replication.
 *
 * On authority it assigns network identities, observes replicated component
 * mutations, builds layouts/snapshots, and publishes them through the selected
 * transport. On clients it validates queued protocol records and reconciles
 * remote Flecs entities. The subsystem owns protocol semantics; transports
 * only move layouts, snapshots, and removals.
 */
UCLASS()
class UNREALFLECS_API UFlecsNetworkWorldSubsystem : public UFlecsAbstractWorldSubsystem
{
	GENERATED_BODY()

public:
	UFlecsNetworkWorldSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnFlecsWorldInitialized(const TSolidNotNull<UFlecsWorld*> InWorld) override;
	virtual void Deinitialize() override;

	/**
	 * Begins authority-side replication for an entity and returns its identity.
	 *
	 * This is idempotent for an entity that already has a valid ID. Adding
	 * FFlecsReplicatedEntityComponent normally invokes it through the built-in
	 * server observer; client calls return an invalid ID.
	 */
	FFlecsNetworkId BeginReplicatingEntity(const FFlecsEntityHandle& EntityHandle);
	/** Stops authority-side replication and publishes a removal when a transport is active. */
	void StopReplicatingEntity(const FFlecsEntityHandle& EntityHandle);
	/** Stops authority-side replication by identity, releasing the allocator slot. */
	void StopReplicatingEntity(FFlecsNetworkId NetworkId);
	/** Marks a replicated entity for its next authoritative full snapshot. */
	void MarkEntityDirty(const FFlecsEntityHandle& EntityHandle);

	/** Enqueues a transport-delivered record for client-side processing on the world tick. */
	void EnqueueReceivedRecord(FFlecsReplicationInboxRecord Record) { Inbox.Enqueue(MoveTemp(Record)); }
	/** Finds the local authoritative entity or client replica currently bound to NetworkId. */
	NO_DISCARD FFlecsEntityHandle FindEntity(FFlecsNetworkId NetworkId) const;

	/** Returns the active provider transport cast to T, or null when no matching transport is active. */
	template <class T = UFlecsReplicationTransportBase>
	NO_DISCARD T* GetReplicationTransport() const { return Cast<T>(ReplicationTransport); }

	/** True for standalone/server/listen-server worlds and false for client worlds. */
	NO_DISCARD bool HasAuthority() const;

#if WITH_AUTOMATION_TESTS
	
	void SetReplicationTransportForTesting(UFlecsReplicationTransportBase* InTransport)
	{
		ReplicationTransport = InTransport;
		
		if (ReplicationTransport)
		{
			ReplicationTransport->InitializeTransport(this);
		}
	}
	
	void FlushServerReplicationForTesting()
	{
		GatherDirtyEntities();
	}
	
	void FlushClientReplicationForTesting()
	{
		DrainInbox();
	}
	
	void EnterClientReplicationModeForTesting()
	{
		bForceClientModeForTesting = true;
	}
	
	void ResetClientReplicationForTesting()
	{
		LayoutRegistry = {};
		DeferredSnapshots.Reset();
		ClientSlotBindings.Reset();
		LastAppliedStateRevisions.Reset();
		EntitySourceShards.Reset();
		EntityPairFixups.Reset();
	}
	
#endif

private:
	/** Local outgoing revision and route state for an authority-side entity. */
	struct FReplicatedEntityState
	{
		uint32 StateRevision = 0;
		uint32 CompositionRevision = 0;
		FFlecsReplicationLayoutId LayoutId;
		FFlecsReplicationRouteKey RouteKey = FFlecsReplicationRouteKey::Default();
	};

	/** Pair waiting for its entity-target replica to become available locally. */
	struct FEntityPairFixup
	{
		FFlecsNetworkId Source;
		FFlecsNetworkId Target;
		FFlecsReplicationKey Key;
	};

	void CreateReplicationTransport();
	void InstallDirtyObservers();
	void InstallDirtyObserversForDescriptor(const FFlecsComponentReplicationDescriptor& Descriptor);
	void GatherDirtyEntities();
	void DrainInbox();
	void ApplySnapshot(const FGuid& SourceShard, const FFlecsReplicatedEntitySnapshot& Snapshot);
	void RemoveRemoteEntity(FFlecsNetworkId NetworkId);
	void DetachRemoteShard(const FGuid& SourceShard);
	void RetryEntityPairFixups();
	bool ResolveKeyToLocalId(const FFlecsReplicationKey& Key, FFlecsId& OutId) const;
	bool ValidateLayout(const FFlecsReplicationLayoutDefinition& Layout, FString& OutError) const;
	void HandleWorldPreActorTick(UWorld* World, ELevelTick TickType, float DeltaSeconds);

	NO_DISCARD TSolidNotNull<const UFlecsNetworkingModuleSettings*> GetNetworkingModuleSettings() const;

	FFlecsNetworkIdAllocator NetworkIdAllocator;
	TMap<FFlecsNetworkId, FFlecsEntityHandle> NetworkIdToEntityHandleMap;
	TMap<FFlecsNetworkId, FReplicatedEntityState> EntityStates;
	TSet<FFlecsNetworkId> DirtyEntities;
	TSet<FString> PublishedLayoutRoutes;
	TArray<flecs::observer> DirtyObservers;
	FFlecsReplicationLayoutRegistry LayoutRegistry;
	FFlecsReplicationInbox Inbox;
	TMap<FFlecsReplicationLayoutId, TArray<TPair<FGuid, FFlecsReplicatedEntitySnapshot>>> DeferredSnapshots;
	TMap<uint32, FFlecsNetworkId> ClientSlotBindings;
	TMap<FFlecsNetworkId, uint32> LastAppliedStateRevisions;
	TMap<FFlecsNetworkId, FGuid> EntitySourceShards;
	TArray<FEntityPairFixup> EntityPairFixups;
	TUniquePtr<IFlecsReplicationRouter> Router;
	FDelegateHandle PreActorTickHandle;
	FDelegateHandle DescriptorRegisteredHandle;

#if WITH_AUTOMATION_TESTS
	bool bForceClientModeForTesting = false;
#endif

	UPROPERTY(Transient)
	TObjectPtr<UFlecsReplicationTransportBase> ReplicationTransport;
};
