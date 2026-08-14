// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Templates/Function.h"

#include "Worlds/FlecsAbstractWorldSubsystem.h"

#include "Networking/FlecsNetworkId.h"
#include "Networking/FlecsReplicationProfile.h"
#include "Networking/FlecsReplicationProfileDataAsset.h"
#include "Networking/FlecsReplicationShardSelection.h"
#include "Networking/FlecsReplicationUpdateQueue.h"
#include "Networking/Layout/FlecsReplicationLayoutRegistry.h"
#include "Networking/Layout/FlecsReplicationSnapshot.h"

#include "FlecsNetworkWorldSubsystem.generated.h"

class UFlecsReplicationBridgeBase;
class IFlecsNetworkIDGeneratorInterface;
class UFlecsNetworkingModuleSettings;

using FFlecsReplicationShardSelectorFunction = TFunction<bool(
	const FFlecsEntityHandle&,
	const FFlecsNetworkId&,
	const FFlecsReplicationProfile&,
	OUT FFlecsReplicationShardSelection&)>;

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
class UNREALFLECSNETWORKING_API UFlecsNetworkWorldSubsystem : public UFlecsAbstractWorldSubsystem
{
	GENERATED_BODY()

public:
	UFlecsNetworkWorldSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnFlecsWorldInitialized(const TSolidNotNull<UFlecsWorld*> InWorld) override;
	virtual void Deinitialize() override;
	
	void RegisterComponentDirtyObservers();
	void RegisterIndividualComponentDirtyObserver(const FFlecsComponentReplicationDescriptor& InDescriptor);
	
	FFlecsNetworkId BeginReplicatingEntity(const FFlecsEntityHandle& InEntityHandle);
	void StopReplicatingEntity(const FFlecsEntityHandle& InEntityHandle);
	
	NO_DISCARD TSolidNotNull<IFlecsNetworkIDGeneratorInterface*> GetNetworkIdGenerator() const;
	
	NO_DISCARD TSolidNotNull<UFlecsReplicationBridgeBase*> GetReplicationBridge() const;

	NO_DISCARD bool HasReplicationBridge() const;

	void BindReplicationBridge(const TSolidNotNull<UFlecsReplicationBridgeBase*> InReplicationBridge);
	void UnbindReplicationBridge(const UFlecsReplicationBridgeBase* InReplicationBridge);

#if WITH_AUTOMATION_TESTS
	void SetReplicationBridgeForTesting(UFlecsReplicationBridgeBase* InReplicationBridge);
#endif // WITH_AUTOMATION_TESTS
	
	template <Solid::TStaticClassConcept T>
	requires (std::is_base_of_v<UFlecsReplicationBridgeBase, T>)
	FORCEINLINE T* GetReplicationBridge() const
	{
		return CastChecked<T>(ReplicationBridge);
	}
	
	NO_DISCARD FORCEINLINE FFlecsReplicationLayoutRegistry& GetLayoutRegistry()
	{
		return LayoutRegistry;
	}
	
	NO_DISCARD FORCEINLINE const FFlecsReplicationLayoutRegistry& GetLayoutRegistry() const
	{
		return LayoutRegistry;
	}
	
	NO_DISCARD FORCEINLINE const TMap<FFlecsNetworkId, FFlecsEntityHandle>& GetNetworkIdToEntityMap() const
	{
		return NetworkIdToEntityMap;
	}
	
	NO_DISCARD FORCEINLINE TMap<FFlecsNetworkId, FFlecsEntityReplicationSnapshot>& GetReplicationSnapshots()
	{
		return ReplicationSnapshots;
	}
	
	NO_DISCARD FORCEINLINE const TMap<FFlecsNetworkId, FFlecsEntityReplicationSnapshot>& GetReplicationSnapshots() const
	{
		return ReplicationSnapshots;
	}
	
	NO_DISCARD bool HasAuthority() const;
	NO_DISCARD bool IsStandalone() const;
	
	template <UE::Flecs::TFlecsEntityHandleTypeConcept T = FFlecsEntityHandle>
	NO_DISCARD TOptional<T> GetEntityFromNetworkId(const FFlecsNetworkId& InNetworkId) const
	{
		if LIKELY_IF(const FFlecsEntityHandle* EntityHandle = NetworkIdToEntityMap.Find(InNetworkId))
		{
			return TOptional<T>(*EntityHandle);
		}
		
		return TOptional<T>();
	}
	
	template <UE::Flecs::TFlecsEntityHandleTypeConcept T = FFlecsEntityHandle>
	NO_DISCARD T GetEntityFromNetworkIdChecked(const FFlecsNetworkId& InNetworkId) const
	{
		if LIKELY_IF(const FFlecsEntityHandle* EntityHandle = NetworkIdToEntityMap.Find(InNetworkId))
		{
			return static_cast<T>(*EntityHandle);
		}
		
		checkf(false, TEXT("No entity found for network ID %s"), *InNetworkId.ToString());
		
		return T();
	}
	
	// @TODO: add a param for if the layout was deferred or not
	void OnEntityLayoutReceived(const FFlecsReplicationLayoutDefinition& InLayout);
	
	void ReceiveNetworkEntitySnapshot(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot);
	void RemoveReceivedNetworkEntity(const FFlecsNetworkId& InNetworkId, uint32 InStateRevision);

	UFUNCTION(BlueprintCallable, Category = "Flecs|Networking")
	FFlecsEntityHandle RegisterReplicationProfileAsset(const UFlecsReplicationProfileDataAsset* InAsset);

	FFlecsEntityHandle RegisterReplicationProfileDefinition(const FName& InName,
		const FFlecsReplicationProfile& InDefinition);

	NO_DISCARD FFlecsEntityHandle GetReplicationProfilePrefab(const FName& InName) const;

	UFUNCTION()
	bool SetReplicationProfile(const FFlecsEntityHandle& InEntity, const FFlecsEntityHandle& InProfilePrefab);

	NO_DISCARD bool ResolveReplicationProfile(const FFlecsEntityHandle& InEntity,
		FFlecsReplicationProfile& OutProfile) const;

	bool RegisterReplicationShardSelector(const FName& InName, FFlecsReplicationShardSelectorFunction InSelector);

	NO_DISCARD bool SelectReplicationShard(const FFlecsEntityHandle& InEntity,
		const FFlecsNetworkId& InNetworkId,
		const FFlecsReplicationProfile& InProfile,
		OUT FFlecsReplicationShardSelection& OutSelection) const;

	void QueueReplicationSnapshot(const FFlecsNetworkId& InNetworkId,
		const FFlecsEntityReplicationSnapshot& InSnapshot);
	void QueueReplicationRemoval(const FFlecsNetworkId& InNetworkId, uint32 InStateRevision);
	void ApplyQueuedReplicationUpdates(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld);

	NO_DISCARD int32 GetQueuedReplicationUpdateCount() const
	{
		return ReplicationUpdateQueue.Num();
	}

protected:
	
	void ApplyReceivedNetworkEntitySnapshot(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot);
	
	void ApplyReceivedNetworkEntityRemoval(const FFlecsNetworkId& InNetworkId, uint32 InStateRevision);
	void ApplyPendingLayoutDefinitions(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld);
	void ApplyDeferredEntityLayouts();
	
	void ApplySnapshotToEntity(const FFlecsEntityHandle& InEntityHandle, const FFlecsEntityReplicationSnapshot& InSnapshot);
	
	// Ran on Client
	void AddDeferredEntityLayout(const FFlecsEntityHandle& InEntityHandle, const FFlecsReplicationLayoutId& InLayout,
		const FFlecsEntityReplicationSnapshot& InSnapshot);
	
	static NO_DISCARD TSolidNotNull<const UFlecsNetworkingModuleSettings*> GetNetworkingSettings();
	
	void CreateReplicationBridge();
	void CreateNetworkIdGenerator();
	
	TMap<FFlecsReplicationLayoutId, TArray<TPair<FFlecsEntityHandle, FFlecsEntityReplicationSnapshot>>> DeferredEntityLayouts;
	
	TMap<FFlecsNetworkId, FFlecsEntityHandle> NetworkIdToEntityMap;
	
	// @TODO: maybe move to a singleton or on the entity?
	TMap<FFlecsNetworkId, FFlecsEntityReplicationSnapshot> ReplicationSnapshots;
	
	// Prevents a late snapshot from resurrecting a removed network entity.
	TMap<FFlecsNetworkId, uint32> RemovedEntityRevisions;

	TMap<FName, FFlecsEntityHandle> ReplicationProfilePrefabs;
	TMap<FName, FFlecsReplicationShardSelectorFunction> ReplicationShardSelectors;
	FFlecsReplicationUpdateQueue ReplicationUpdateQueue;
	
	UPROPERTY()
	TArray<FFlecsObserverHandle> ComponentDirtyObservers;
	
	UPROPERTY()
	TObjectPtr<UObject> NetworkIdGenerator;
	
	UPROPERTY()
	TObjectPtr<UFlecsReplicationBridgeBase> ReplicationBridge;
	
	FFlecsReplicationLayoutRegistry LayoutRegistry;
}; // class UFlecsNetworkWorldSubsystem
