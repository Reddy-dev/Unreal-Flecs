// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"
#include "Net/Core/PushModel/PushModelMacros.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectAdapter.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectFactory.h"

#include "FlecsLayoutReplicatorFastArray.h"

#include "FlecsLayoutReplicator.generated.h"

class UFlecsReplicationBridgeBase;

/**
 * Always-relevant catalogue of immutable Flecs table layouts.
 *
 * Layouts are retained for late joiners for the lifetime of the replicator.
 */
UCLASS()
class UNREALFLECS_API UFlecsLayoutReplicator : public UObject, public INetRootObjectFactoryExtension
{
	GENERATED_BODY()
	REPLICATED_BASE_CLASS(UFlecsLayoutReplicator)

public:
	UFlecsLayoutReplicator();
	virtual void PostInitProperties() override;

	virtual bool IsSupportedForNetworking() const override
	{
		return true;
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void RegisterReplicationFragments(
		UE::Net::FFragmentRegistrationContext& Fragments,
		UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
	virtual void FillRootObjectReplicationParams(
		const UE::Net::FRootObjectReplicationParamsContext& Context,
		UE::Net::FRootObjectReplicationParams& OutParams) const override;

	void InitializeReplicator(UFlecsReplicationBridgeBase* InReplicationBridge);
	void DeinitializeReplicator();
	NO_DISCARD bool TryStartReplication();

	/** Rebinds a factory-created client object and replays its retained layouts. */
	void BindReplicationBridge(UFlecsReplicationBridgeBase* InReplicationBridge);

	void PublishLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition);

	void ReceiveLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition);

	NO_DISCARD const FFlecsReplicatorFastArray& GetReplicatedLayouts() const
	{
		return ReplicatedLayouts;
	}

private:
	UPROPERTY(Replicated)
	FFlecsReplicatorFastArray ReplicatedLayouts;

	UPROPERTY(Transient)
	TWeakObjectPtr<UFlecsReplicationBridgeBase> ReplicationBridge;

	UE::Net::FNetRootObjectAdapter RootObjectAdapter;

}; // class UFlecsLayoutReplicator
