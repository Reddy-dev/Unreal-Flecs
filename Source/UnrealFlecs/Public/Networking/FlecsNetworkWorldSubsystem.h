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
 * mutations, builds layouts/updates, and publishes them through the selected
 * transport. On clients it validates queued protocol records and reconciles
 * remote Flecs entities. The subsystem owns protocol semantics; transports
 * only move layouts, updates, and removals.
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
	
	/** Marks all payload keys dirty; structural changes still force a full update. */
	void MarkEntityDirty(const FFlecsEntityHandle& EntityHandle);

	/** Marks one component or concrete pair dirty after an in-place mutation. */
	void MarkComponentDirty(const FFlecsEntityHandle& EntityHandle, FFlecsId ComponentOrPairId);

	template <typename T>
	void MarkComponentDirty(const FFlecsEntityHandle& EntityHandle)
	{
		if (IsFlecsWorldValid())
		{
			MarkComponentDirty(EntityHandle, GetFlecsWorldChecked()->GetIdIfRegistered<T>());
		}
	}

	/** Replaces the route-selection policy. Passing null restores the default router. */
	void SetReplicationRouter(TUniquePtr<IFlecsReplicationRouter> InRouter);

	/** Inserts or replaces one policy-owned fragment for a connection. */
	template <typename TFragment>
	void SetConnectionInterestFragment(const uint32 ConnectionId, const TFragment& InFragment)
	{
		ConnectionInterestContexts.FindOrAdd(ConnectionId).Set<TFragment>(InFragment);
	}

	/** Removes one fragment type and reclaims an empty connection context. */
	template <typename TFragment>
	bool RemoveConnectionInterestFragment(const uint32 ConnectionId)
	{
		FFlecsReplicationConnectionInterestContext* Context = ConnectionInterestContexts.Find(ConnectionId);
		if (!Context || !Context->Remove<TFragment>())
		{
			return false;
		}
		if (Context->IsEmpty())
		{
			ConnectionInterestContexts.Remove(ConnectionId);
		}
		return true;
	}

	/** Removes all policy-owned state for a disconnected or reset connection. */
	void ClearConnectionInterestContext(uint32 ConnectionId);
	/** Validates registration, exact descriptor type, stability, and policy-specific constraints. */
	NO_DISCARD bool ValidateInterestBinding(const FFlecsReplicationInterestBinding& Binding,
		FString& OutError) const;
	/** Dispatches one policy binding with the current typed context and transport-neutral view. */
	NO_DISCARD bool IsInterestBindingRelevant(const FFlecsReplicationInterestBinding& Binding,
		FName RouteName, uint32 ConnectionId, const FFlecsReplicationConnectionView& View) const;
	/** Evaluates a page descriptor against connection context and its current view. */
	NO_DISCARD bool IsRouteRelevant(const FFlecsReplicationRouteDescriptor& Route, uint32 ConnectionId,
		const FFlecsReplicationConnectionView& View) const;
	/** Selects the entity's dormancy behavior. Dirty structure or state always wakes it for a flush. */
	void SetReplicationDormancy(const FFlecsEntityHandle& EntityHandle, EFlecsReplicationDormancyMode Mode);

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
		DeferredUpdates.Reset();
		ClientSlotBindings.Reset();
		LastAppliedStateRevisions.Reset();
		LastAppliedLayoutIds.Reset();
		EntitySourceShards.Reset();
		EntityPairFixups.Reset();
		MaterializedRemoteUpdates.Reset();
		RemoteLayoutSources.Reset();
		UpdateReassembler.Reset();
	}

	void SetReplicationTimeForTesting(double InSeconds)
	{
		TestingTimeSeconds = InSeconds;
	}

	void SetPayloadBudgetForTesting(uint32 InBytes)
	{
		TestingPayloadBudget = InBytes;
	}
	
#endif

private:
	/** Local outgoing revision and route state for an authority-side entity. */
	struct FReplicatedEntityState
	{
		uint32 StateRevision = 0;
		uint32 CompositionRevision = 0;
		FFlecsReplicationLayoutId LayoutId;
		FFlecsReplicationRouteDescriptor Route = FFlecsReplicationRouteDescriptor::Default();
		TMap<uint16, TArray<uint8>> CanonicalValues;
		TMap<uint16, TArray<uint8>> LastSentCanonicalValues;
		TMap<uint16, TArray<uint8>> LastSentEncodedValues;
		TMap<uint16, TArray<uint8>> PendingEncodedPayloads;
		TMap<FFlecsReplicationSchemaId, double> LastSendTimes;
		TSet<FFlecsId> DirtyComponentIds;
		EFlecsReplicationDormancyMode DormancyMode = EFlecsReplicationDormancyMode::Automatic;
		double LastDirtyTime = 0.0;
		bool bAllPayloadDirty = true;
		bool bNeedsFullUpdate = true;
		bool bRouteInterestValid = false;
		bool bDormant = false;
	};

	struct FPublishedLayoutKey
	{
		FFlecsReplicationRouteDescriptor Route;
		FFlecsReplicationLayoutId LayoutId;

		friend bool operator==(const FPublishedLayoutKey&, const FPublishedLayoutKey&) = default;
	}; // struct FPublishedLayoutKey

	/** Complete pair state from one accepted update, waiting for its entity-target replica. */
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
	void ApplyUpdate(const FGuid& SourceShard, const FFlecsReplicatedEntityUpdate& Update);
	void ApplyMaterializedUpdate(const FGuid& SourceShard, const FFlecsReplicatedEntityUpdate& Update);
	void RemoveRemoteEntity(FFlecsNetworkId NetworkId, const FGuid* ExpectedSource = nullptr);
	void DetachRemoteShard(const FGuid& SourceShard);
	void TryReclaimRemoteLayout(FFlecsReplicationLayoutId LayoutId);
	void RetryEntityPairFixups();
	void ApplyResolvedValue(const FFlecsEntityHandle& Entity, FFlecsId LocalId,
		const FFlecsReplicationKey& Key, const TArray<uint8>* Payload) const;
	bool ResolveKeyToLocalId(const FFlecsReplicationKey& Key, FFlecsId& OutId) const;
	bool ValidateLayout(const FFlecsReplicationLayoutDefinition& Layout, FString& OutError) const;
	void ReportInvalidInterestBinding(const FFlecsReplicationRouteDescriptor& Route,
		const FString& Error, const TCHAR* Source) const;
	void HandleWorldPreActorTick(UWorld* World, ELevelTick TickType, float DeltaSeconds);
	NO_DISCARD double GetReplicationTimeSeconds() const;

	NO_DISCARD TSolidNotNull<const UFlecsNetworkingModuleSettings*> GetNetworkingModuleSettings() const;

	FFlecsNetworkIdAllocator NetworkIdAllocator;
	
	UPROPERTY(Transient)
	TMap<FFlecsNetworkId, FFlecsEntityHandle> NetworkIdToEntityHandleMap;
	
	TMap<FFlecsNetworkId, FReplicatedEntityState> EntityStates;
	
	UPROPERTY(Transient)
	TSet<FFlecsNetworkId> DirtyEntities;
	
	TArray<FPublishedLayoutKey> PublishedLayoutRoutes;
	
	UPROPERTY()
	TArray<FFlecsObserverHandle> DirtyObservers;
	
	FFlecsReplicationLayoutRegistry LayoutRegistry;
	FFlecsReplicationInbox Inbox;
	
	TMap<FFlecsReplicationLayoutId, TArray<TPair<FGuid, FFlecsReplicatedEntityUpdate>>> DeferredUpdates;
	TMap<FFlecsNetworkId, FFlecsReplicatedEntityUpdate> MaterializedRemoteUpdates;
	FFlecsReplicationUpdateReassembler UpdateReassembler;
	
	UPROPERTY()
	TMap<uint32, FFlecsNetworkId> ClientSlotBindings;
	
	UPROPERTY()
	TMap<FFlecsNetworkId, uint32> LastAppliedStateRevisions;
	TMap<FFlecsNetworkId, FFlecsReplicationLayoutId> LastAppliedLayoutIds;
	
	TMap<FFlecsNetworkId, FGuid> EntitySourceShards;
	TMap<FFlecsReplicationLayoutId, TSet<FGuid>> RemoteLayoutSources;
	
	TArray<FEntityPairFixup> EntityPairFixups;
	
	TUniquePtr<IFlecsReplicationRouter> Router;
	TMap<uint32, FFlecsReplicationConnectionInterestContext> ConnectionInterestContexts;
	mutable TSet<FString> LoggedInvalidInterestBindings;
	
	FDelegateHandle PreActorTickHandle;
	
	FDelegateHandle DescriptorRegisteredHandle;

#if WITH_AUTOMATION_TESTS || WITH_EDITOR
	bool bForceClientModeForTesting = false;
	TOptional<double> TestingTimeSeconds;
	TOptional<uint32> TestingPayloadBudget;
#endif

	UPROPERTY(Transient)
	TObjectPtr<UFlecsReplicationTransportBase> ReplicationTransport;
}; // class UFlecsNetworkWorldSubsystem
