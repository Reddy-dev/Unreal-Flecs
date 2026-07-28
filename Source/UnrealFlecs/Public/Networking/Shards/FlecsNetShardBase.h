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
#include "Networking/Router/FlecsNetRouteId.h"

#include "FlecsNetShardBase.generated.h"

class UFlecsReplicationBridgeBase;
/**
 * Generic replicated storage object addressed through a bridge route.
 *
 * Concrete shards define the stored payload. The bridge only depends on this
 * base class and resolves the concrete storage object by RouteId.
 */
UCLASS()
class UNREALFLECS_API UFlecsNetShardBase : public UObject, public INetRootObjectFactoryExtension
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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Fragments,
		UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
	virtual void FillRootObjectReplicationParams(const UE::Net::FRootObjectReplicationParamsContext& Context,
	                                             UE::Net::FRootObjectReplicationParams& OutParams) const override;
	
	virtual void ConfigureObjectSettings(OUT UE::Net::FRootObjectSettings& OutSettings) const;

	NO_DISCARD const FFlecsNetRouteId& GetRouteId() const
	{
		return RouteId;
	}
	
	virtual void PublishNetEntity(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot)
		PURE_VIRTUAL(UFlecsNetShardBase::PublishNetEntity, );
	
	void SetOwningNetworkWorldSubsystem(UFlecsNetworkWorldSubsystem* InOwningNetworkWorldSubsystem);
	
	NO_DISCARD UFlecsNetworkWorldSubsystem* GetOwningNetworkWorldSubsystem() const;
	
	void SetRouteId(const FFlecsNetRouteId& InRouteId);
	
protected:
	
	void ReceiveEntityUpdate(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot);

private:

	UPROPERTY(Replicated)
	FFlecsNetRouteId RouteId = FFlecsNetRouteId::Default();
	
	UPROPERTY()
	TWeakObjectPtr<UFlecsNetworkWorldSubsystem> OwningNetworkWorldSubsystem;
	
	UE::Net::FNetRootObjectAdapter RootObjectAdapter;

}; // class UFlecsNetShardBase
