// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Net/Core/PushModel/PushModelMacros.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectAdapter.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectFactory.h"

#include "Networking/Bridge/FlecsReplicationBridgeBase.h"
#include "Layout/FlecsLayoutReplicatorFastArray.h"

#include "FlecsIrisReplicationBridge.generated.h"

class UWorld;

/**
 * Always-relevant Iris root object coordinating Flecs replication.
 */
UCLASS()
class UNREALFLECS_API UFlecsIrisReplicationBridge : public UFlecsReplicationBridgeBase, public INetRootObjectFactoryExtension
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

	bool TryStartReplication();

	virtual void PublishEntityLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition) override;

	void ReceiveLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition);
	
	virtual void PublishNetEntity(const FFlecsNetRouteId& InRouteId,
		const FFlecsNetworkId& InNetworkId,
		const FFlecsEntityReplicationSnapshot& InSnapshot) override;
	
	//virtual UFlecsNetShardBase* ResolveShard(const FFlecsNetRouteId& InRouteId, const FFlecsEntityHandle& InEntityHandle);

	NO_DISCARD const FFlecsReplicatorFastArray& GetReplicatedLayouts() const
	{
		return ReplicatedLayouts;
	}
	
protected:
	void HandleWorldPreActorTick(UWorld* InWorld, ELevelTick InTickType, float InDeltaSeconds);

	UPROPERTY(Replicated)
	FFlecsReplicatorFastArray ReplicatedLayouts;

	UPROPERTY()
	TMap<FFlecsNetRouteId, TObjectPtr<UFlecsNetShardBase>> ShardMap;

	FDelegateHandle WorldPreActorTickHandle;

	UE::Net::FNetRootObjectAdapter RootObjectAdapter;
	
}; // class UFlecsIrisReplicationBridge
