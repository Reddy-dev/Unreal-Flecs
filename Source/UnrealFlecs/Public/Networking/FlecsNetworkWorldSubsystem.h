// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsNetworkId.h"
#include "FlecsReplicationTypes.h"
#include "Worlds/FlecsAbstractWorldSubsystem.h"

#include "FlecsNetworkWorldSubsystem.generated.h"

class UFlecsReplicationBridgeBase;
class IFlecsNetworkIDGeneratorInterface;
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
	
	void RegisterComponentDirtyObservers();
	void RegisterIndividualComponentDirtyObserver(const FFlecsComponentReplicationDescriptor& InDescriptor);
	
	FFlecsNetworkId BeginReplicatingEntity(const FFlecsEntityHandle& InEntityHandle);
	void StopReplicatingEntity(const FFlecsEntityHandle& InEntityHandle);
	
	NO_DISCARD TSolidNotNull<IFlecsNetworkIDGeneratorInterface*> GetNetworkIdGenerator() const;
	NO_DISCARD TSolidNotNull<UFlecsReplicationBridgeBase*> GetReplicationBridge() const;
	
	template <Solid::TStaticClassConcept T>
	requires (std::is_base_of_v<UFlecsReplicationBridgeBase, T>)
	FORCEINLINE T* GetReplicationBridge() const
	{
		return CastChecked<T>(ReplicationBridge);
	}

protected:
	NO_DISCARD bool HasAuthority() const;
	NO_DISCARD bool IsStandalone() const;
	
	static NO_DISCARD TSolidNotNull<const UFlecsNetworkingModuleSettings*> GetNetworkingSettings();
	
	void CreateReplicationBridge();
	void CreateNetworkIdGenerator();
	
	TMap<FFlecsNetworkId, FFlecsEntityHandle> NetworkIdToEntityMap;
	
	
	UPROPERTY()
	TArray<FFlecsObserverHandle> ComponentDirtyObservers;
	
	UPROPERTY()
	TObjectPtr<UObject> NetworkIdGenerator;
	
	UPROPERTY()
	TObjectPtr<UFlecsReplicationBridgeBase> ReplicationBridge;
	
}; // class UFlecsNetworkWorldSubsystem
