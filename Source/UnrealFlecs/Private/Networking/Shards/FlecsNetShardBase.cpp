// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Shards/FlecsNetShardBase.h"

#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetShardBase)

void UFlecsNetShardBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams LifetimeParams;
	LifetimeParams.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsNetShardBase, RouteId, LifetimeParams);
}

void UFlecsNetShardBase::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Fragments,
	UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Fragments, RegistrationFlags);
}

void UFlecsNetShardBase::FillRootObjectReplicationParams(const UE::Net::FRootObjectReplicationParamsContext& Context,
	UE::Net::FRootObjectReplicationParams& OutParams) const
{
	OutParams.
}

UFlecsReplicationBridgeBase* UFlecsNetShardBase::GetReplicationBridge() const
{
	return GetTypedOuter<UFlecsReplicationBridgeBase>();
}

void UFlecsNetShardBase::SetRouteId(const FFlecsNetRouteId& InRouteId)
{
	RouteId = InRouteId;
	MARK_PROPERTY_DIRTY_FROM_NAME(UFlecsNetShardBase, RouteId, this);
}
