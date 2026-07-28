// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsNetEntityProxy.h"

#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetEntityProxy)

void UFlecsNetEntityProxy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsNetEntityProxy, NetworkId, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsNetEntityProxy, Snapshot, Params);
}

void UFlecsNetEntityProxy::PublishNetEntity(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot)
{
	NetworkId = InNetworkId;
	Snapshot = InSnapshot;
	
	MARK_PROPERTY_DIRTY_FROM_NAME(UFlecsNetEntityProxy, NetworkId, this);
	MARK_PROPERTY_DIRTY_FROM_NAME(UFlecsNetEntityProxy, Snapshot, this);
}

void UFlecsNetEntityProxy::OnRep_NetworkId()
{
	ReceiveEntityUpdate(NetworkId, Snapshot);
}

void UFlecsNetEntityProxy::OnRep_Snapshot()
{
	ReceiveEntityUpdate(NetworkId, Snapshot);
}
