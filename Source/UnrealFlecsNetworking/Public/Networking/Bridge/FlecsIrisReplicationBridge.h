// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Net/Core/PushModel/PushModelMacros.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectAdapter.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectFactory.h"

#include "Networking/Bridge/FlecsReplicationBridgeBase.h"
#include "Networking/FlecsReplicationProfile.h"
#include "Networking/FlecsReplicationShardSelection.h"
#include "Networking/Layout/FlecsLayoutReplicatorFastArray.h"

#include "FlecsIrisReplicationBridge.generated.h"

/**
 * Always-relevant Iris root object coordinating Flecs replication.
 */
USTRUCT()
struct FFlecsReplicationShardPlacement
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UFlecsNetShardBase> Shard = nullptr;
	UPROPERTY()
	FFlecsReplicationProfile Profile;
	FFlecsReplicationShardSelection Selection;
	UPROPERTY()
	uint32 TargetGeneration = 0;
	UPROPERTY()
	uint32 PlacementGeneration = 0;

}; // struct FFlecsReplicationShardPlacement

UCLASS()
class UNREALFLECSNETWORKING_API UFlecsIrisReplicationBridge : public UFlecsReplicationBridgeBase, public INetRootObjectFactoryExtension
{
	GENERATED_BODY()
	REPLICATED_BASE_CLASS(UFlecsIrisReplicationBridge)

public:
	UFlecsIrisReplicationBridge(const FObjectInitializer& ObjectInitializer);
	virtual void PostInitProperties() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Fragments,
		UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
	virtual void FillRootObjectReplicationParams(const UE::Net::FRootObjectReplicationParamsContext& Context,
		UE::Net::FRootObjectReplicationParams& OutParams) const override;
	
	virtual void InitializeBridge() override;
	virtual void DeinitializeBridge() override;

	virtual void PublishEntityLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition) override;

	void ReceiveLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition);
	
	virtual void PublishNetEntity(const FFlecsEntityHandle& EntityHandle, const FFlecsNetworkId& InNetworkId,
		const FFlecsEntityReplicationSnapshot& InSnapshot);
	virtual void StopReplicatingEntity(const FFlecsEntityHandle& InEntityHandle) override;
	
	virtual NO_DISCARD UFlecsNetShardBase* ResolveShard(const FFlecsEntityHandle& InEntityHandle,
		const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot);

	NO_DISCARD const FFlecsReplicatorFastArray& GetReplicatedLayouts() const
	{
		return ReplicatedLayouts;
	}
	
protected:
	NO_DISCARD UFlecsNetShardBase* CreateNewShard(const FFlecsNetworkId& InNetworkId,
		const FFlecsReplicationProfile& InProfile,
		const FFlecsReplicationShardSelection& InSelection);

	UPROPERTY(Replicated)
	FFlecsReplicatorFastArray ReplicatedLayouts;

	UPROPERTY()
	TMap<FFlecsEntityView, FFlecsReplicationShardPlacement> ShardMap;

	UE::Net::FNetRootObjectAdapter RootObjectAdapter;
	
}; // class UFlecsIrisReplicationBridge
