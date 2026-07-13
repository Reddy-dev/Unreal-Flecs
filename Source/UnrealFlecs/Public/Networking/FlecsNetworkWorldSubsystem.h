// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Worlds/FlecsAbstractWorldSubsystem.h"
#include "Networking/FlecsReplicationTransportBase.h"

#include "FlecsNetworkWorldSubsystem.generated.h"

class UFlecsNetworkingModuleSettings;

UCLASS()
class UNREALFLECS_API UFlecsNetworkWorldSubsystem : public UFlecsAbstractWorldSubsystem
{
	GENERATED_BODY()

public:
	UFlecsNetworkWorldSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnFlecsWorldInitialized(const TSolidNotNull<UFlecsWorld*> InWorld) override;
	virtual void Deinitialize() override;

	FFlecsNetworkId BeginReplicatingEntity(const FFlecsEntityHandle& EntityHandle);
	void StopReplicatingEntity(const FFlecsEntityHandle& EntityHandle);
	void StopReplicatingEntity(FFlecsNetworkId NetworkId);
	void MarkEntityDirty(const FFlecsEntityHandle& EntityHandle);

	void EnqueueReceivedRecord(FFlecsReplicationInboxRecord Record) { Inbox.Enqueue(MoveTemp(Record)); }
	NO_DISCARD FFlecsEntityHandle FindEntity(FFlecsNetworkId NetworkId) const;

	template <class T = UFlecsReplicationTransportBase>
	NO_DISCARD T* GetReplicationTransport() const { return Cast<T>(ReplicationTransport); }

	NO_DISCARD bool HasAuthority() const;

#if WITH_AUTOMATION_TESTS
	void SetReplicationTransportForTesting(UFlecsReplicationTransportBase* InTransport)
	{
		ReplicationTransport = InTransport;
		if (ReplicationTransport) ReplicationTransport->InitializeTransport(this);
	}
	void FlushServerReplicationForTesting() { GatherDirtyEntities(); }
	void FlushClientReplicationForTesting() { DrainInbox(); }
	void EnterClientReplicationModeForTesting() { bForceClientModeForTesting = true; }
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
	struct FReplicatedEntityState
	{
		uint32 StateRevision = 0;
		uint32 CompositionRevision = 0;
		FFlecsReplicationLayoutId LayoutId;
		FFlecsReplicationRouteKey RouteKey = FFlecsReplicationRouteKey::Default();
	};

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
