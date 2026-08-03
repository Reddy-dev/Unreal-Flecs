// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"
#include "Net/Core/PushModel/PushModelMacros.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "Net/Iris/ReplicationSystem/NetRootObjectAdapter.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectFactory.h"
#include "Networking/FlecsNetworkId.h"
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
	
	virtual void PublishNetEntity(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot)
		PURE_VIRTUAL(UFlecsNetShardBase::PublishNetEntity, );

	void SetOwningNetworkWorldSubsystem(UFlecsNetworkWorldSubsystem* InOwningNetworkWorldSubsystem);

	NO_DISCARD UFlecsNetworkWorldSubsystem* GetOwningNetworkWorldSubsystem() const;

protected:
	void ReceiveEntityUpdate(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot);

private:

	UPROPERTY()
	TWeakObjectPtr<UFlecsNetworkWorldSubsystem> OwningNetworkWorldSubsystem;

	UE::Net::FNetRootObjectAdapter RootObjectAdapter;

}; // class UFlecsNetShardBase
