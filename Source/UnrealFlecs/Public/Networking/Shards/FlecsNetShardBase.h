// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"
#include "Net/Core/PushModel/PushModelMacros.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "FlecsNetRouteId.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectFactory.h"
#include "Networking/FlecsNetworkId.h"
#include "Networking/Layout/FlecsReplicationSnapshot.h"

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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Fragments,
		UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
	virtual void FillRootObjectReplicationParams(const UE::Net::FRootObjectReplicationParamsContext& Context, UE::Net::FRootObjectReplicationParams& OutParams) const override;

	NO_DISCARD const FFlecsNetRouteId& GetRouteId() const
	{
		return RouteId;
	}
	
	NO_DISCARD UFlecsReplicationBridgeBase* GetReplicationBridge() const;
	
	template <typename T>
	NO_DISCARD T* GetReplicationBridge() const
	{
		return Cast<T>(GetReplicationBridge());
	}
	
	template <typename T>
	NO_DISCARD TSolidNotNull<T*> GetReplicationBridgeChecked() const
	{
		return CastChecked<T>(GetReplicationBridge());
	}
	
	virtual void PublishNetEntity(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot)
		PURE_VIRTUAL(UFlecsNetShardBase::PublishNetEntity, );
	

private:
	friend class UFlecsReplicationBridgeBase;

	void SetRouteId(const FFlecsNetRouteId& InRouteId);

	UPROPERTY(Replicated)
	FFlecsNetRouteId RouteId = FFlecsNetRouteId::Default();

}; // class UFlecsNetShardBase
