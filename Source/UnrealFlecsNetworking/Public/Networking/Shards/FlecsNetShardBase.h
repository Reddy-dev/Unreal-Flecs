// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/World.h"
#include "UObject/Object.h"
#include "Net/Core/PushModel/PushModelMacros.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "Net/Iris/ReplicationSystem/NetRootObjectAdapter.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectFactory.h"
#include "Networking/FlecsNetworkId.h"
#include "Networking/FlecsReplicationUpdateQueue.h"
#include "Networking/Layout/FlecsReplicationSnapshot.h"

#include "FlecsNetShardBase.generated.h"

class UFlecsNetworkWorldSubsystem;
struct FFlecsReplicationProfile;

/**
 * Generic replicated storage object selected by the replication bridge.
 *
 * Concrete shards define the stored payload. The bridge only depends on this
 * base class and resolves the concrete storage object from the entity profile.
 */
UCLASS()
class UNREALFLECSNETWORKING_API UFlecsNetShardBase : public UObject, public INetRootObjectFactoryExtension
{
	GENERATED_BODY()
	REPLICATED_BASE_CLASS(UFlecsNetShardBase)

public:

	virtual bool IsSupportedForNetworking() const override
	{
		return true;
	}

	virtual UWorld* GetWorld() const override;

	virtual void InitializeShard();
	virtual void DeinitializeShard();
	
	void StartShardReplication();
	void StopShardReplication();

	NO_DISCARD bool IsShardReplicating() const
	{
		return RootObjectAdapter.IsReplicating();
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Fragments,
		UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
	
	virtual void FillRootObjectReplicationParams(const UE::Net::FRootObjectReplicationParamsContext& Context,
	                                             UE::Net::FRootObjectReplicationParams& OutParams) const override;

	virtual void ConfigureObjectSettings(OUT UE::Net::FRootObjectSettings& OutSettings) const;
	bool ApplyReplicationProfile(const FFlecsReplicationProfile& InProfile);
	
	/** Returns whether this shard can store an update for the supplied entity. */
	virtual bool CanAcceptNetEntity(const FFlecsNetworkId& InNetworkId,
		const FFlecsEntityReplicationSnapshot& InSnapshot) const
		PURE_VIRTUAL(UFlecsNetShardBase::CanAcceptNetEntity, return false;);

	/** Inserts or replaces the replicated state for one entity. */
	virtual void PublishNetEntity(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot)
		PURE_VIRTUAL(UFlecsNetShardBase::PublishNetEntity, );

	/** Removes one replicated entity without tearing down the physical shard. */
	virtual void RemoveNetEntity(const FFlecsNetworkId& InNetworkId)
		PURE_VIRTUAL(UFlecsNetShardBase::RemoveNetEntity, );

	/** Whether the physical shard contains no replicated entities. */
	virtual bool IsEmpty() const
		PURE_VIRTUAL(UFlecsNetShardBase::IsEmpty, return true;);
	
	virtual NO_DISCARD TOptional<UNetObjectFactory::FWorldInfoData> GetWorldInfoData() const;

	/** Assigns the local world even when this dynamic Iris root has a transient outer. */
	void SetOwningWorld(const TSolidNotNull<UWorld*> InOwningWorld);

	/** Binds the local networking subsystem and flushes any early received state. */
	void SetOwningNetworkWorldSubsystem(UFlecsNetworkWorldSubsystem* InOwningNetworkWorldSubsystem);

	NO_DISCARD UFlecsNetworkWorldSubsystem* GetOwningNetworkWorldSubsystem() const;

protected:
	void ReceiveEntityUpdate(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot);
	void ReceiveEntityRemoval(const FFlecsNetworkId& InNetworkId, uint32 InStateRevision);

	void ResolveOwningNetworkWorldSubsystem();
	void HandleWorldPreActorTick(UWorld* InWorld, ELevelTick, float);
	void StartOwningNetworkWorldSubsystemRetry();
	void StopOwningNetworkWorldSubsystemRetry();
	void FlushPendingReplicationUpdates();

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UWorld> OwningWorld;

	UPROPERTY()
	TWeakObjectPtr<UFlecsNetworkWorldSubsystem> OwningNetworkWorldSubsystem;

	FFlecsReplicationUpdateQueue PendingReplicationUpdateQueue;
	FDelegateHandle WorldPreActorTickHandle;

	UE::Net::FNetRootObjectAdapter RootObjectAdapter;

}; // class UFlecsNetShardBase
