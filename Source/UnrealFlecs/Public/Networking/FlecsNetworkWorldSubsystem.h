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
	void EnqueueReceivedRecord(FFlecsReplicationInboxRecord Record)
	{
		Inbox.Enqueue(MoveTemp(Record));
	}
	
	/** Finds the local authoritative entity or client replica currently bound to NetworkId. */
	NO_DISCARD FFlecsEntityHandle FindEntity(FFlecsNetworkId NetworkId) const;

	/** Returns the active provider transport cast to T, or null when no matching transport is active. */
	template <class T = UFlecsReplicationTransportBase>
	NO_DISCARD FORCEINLINE T* GetReplicationTransport() const
	{
		return Cast<T>(ReplicationTransport);
	}
	
	template <class T = UFlecsReplicationTransportBase>
	NO_DISCARD FORCEINLINE TSolidNotNull<T*> GetReplicationTransportChecked() const
	{
		return CastChecked<T>(ReplicationTransport);
	}

	/** True for standalone/server/listen-server worlds and false for client worlds. */
	NO_DISCARD bool HasAuthority() const;

#if WITH_AUTOMATION_TESTS || WITH_EDITOR
	
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

	/** Complete pair state from one source snapshot, waiting for its entity-target replica. */
	struct FEntityPairFixup
	{
		FFlecsNetworkId Source;
		FFlecsNetworkId Target;
		FFlecsReplicationKey Key;
		uint32 StateRevision = 0;
		TOptional<TArray<uint8>> Payload;
	}; // struct FEntityPairFixup

	void CreateReplicationTransport();
	
	void InstallDirtyObservers();
	void InstallDirtyObserversForDescriptor(const FFlecsComponentReplicationDescriptor& Descriptor);
	
	void GatherDirtyEntities();
	void DrainInbox();
	
	void ApplySnapshot(const FGuid& SourceShard, const FFlecsReplicatedEntitySnapshot& Snapshot);
	void RemoveRemoteEntity(FFlecsNetworkId NetworkId);
	void DetachRemoteShard(const FGuid& SourceShard);
	
	void RetryEntityPairFixups();
	void ApplyResolvedValue(const FFlecsEntityHandle& Entity, FFlecsId LocalId,
		const FFlecsReplicationKey& Key, const TArray<uint8>* Payload) const;
	
	bool ResolveIndividualKeyToLocalId(const FFlecsReplicationIndividualKey& Key, OUT FFlecsId& OutId) const;
	bool ResolveKeyToLocalId(const FFlecsReplicationKey& Key, OUT FFlecsId& OutId) const;
	
	NO_DISCARD bool ValidateReplicationIndividualKey(const FFlecsReplicationIndividualKey& Key, OUT FString& OutError) const;
	NO_DISCARD bool ValidateReplicationKey(const FFlecsReplicationKey& Key, OUT FString& OutError) const;
	NO_DISCARD bool ValidateLayout(const FFlecsReplicationLayoutDefinition& Layout, OUT FString& OutError) const;
	
	void HandleWorldPreActorTick(UWorld* World, ELevelTick TickType, float DeltaSeconds);

	NO_DISCARD TSolidNotNull<const UFlecsNetworkingModuleSettings*> GetNetworkingModuleSettings() const;

	FFlecsNetworkIdAllocator NetworkIdAllocator;
	
	UPROPERTY(Transient)
	TMap<FFlecsNetworkId, FFlecsEntityHandle> NetworkIdToEntityHandleMap;
	
	TMap<FFlecsNetworkId, FReplicatedEntityState> EntityStates;
	
	UPROPERTY(Transient)
	TSet<FFlecsNetworkId> DirtyEntities;
	
	UPROPERTY()
	TSet<FString> PublishedLayoutRoutes;
	
	UPROPERTY()
	TArray<FFlecsObserverHandle> DirtyObservers;
	
	FFlecsReplicationLayoutRegistry LayoutRegistry;
	FFlecsReplicationInbox Inbox;
	
	TMap<FFlecsReplicationLayoutId, TArray<TPair<FGuid, FFlecsReplicatedEntitySnapshot>>> DeferredSnapshots;
	
	UPROPERTY()
	TMap<uint32, FFlecsNetworkId> ClientSlotBindings;
	
	UPROPERTY()
	TMap<FFlecsNetworkId, uint32> LastAppliedStateRevisions;
	
	TMap<FFlecsNetworkId, FGuid> EntitySourceShards;
	
	TArray<FEntityPairFixup> EntityPairFixups;
	
	TUniquePtr<IFlecsReplicationRouter> Router;
	
	FDelegateHandle PreActorTickHandle;
	
	FDelegateHandle DescriptorRegisteredHandle;

#if WITH_AUTOMATION_TESTS || WITH_EDITOR
	bool bForceClientModeForTesting = false;
#endif

	UPROPERTY(Transient)
	TObjectPtr<UFlecsReplicationTransportBase> ReplicationTransport;
}; // class UFlecsNetworkWorldSubsystem
