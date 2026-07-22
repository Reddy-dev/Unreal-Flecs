// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsNetworkId.h"
#include "FlecsReplicationTypes.h"
#include "Worlds/FlecsAbstractWorldSubsystem.h"

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
	
	void RegisterComponentDirtyObservers();
	void RegisterIndividualComponentDirtyObserver(const FFlecsComponentReplicationDescriptor& InDescriptor);
	
	FFlecsNetworkId BeginReplicatingEntity(const FFlecsEntityHandle& InEntity);
	void StopReplicatingEntity(const FFlecsEntityHandle& InEntity);

protected:
	NO_DISCARD bool HasAuthority() const;
	NO_DISCARD bool IsStandalone() const;
	
	UPROPERTY()
	TArray<FFlecsObserverHandle> ComponentDirtyObservers;
	
	
	
}; // class UFlecsNetworkWorldSubsystem
