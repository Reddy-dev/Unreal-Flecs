// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Shards/FlecsNetEntityProxy.h"

#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

#include "Networking/Shards/FlecsNetEntityProxyNetFactory.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetEntityProxy)

void UFlecsNetEntityProxy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsNetEntityProxy, NetworkId, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsNetEntityProxy, Snapshot, Params);
}

void UFlecsNetEntityProxy::ConfigureObjectSettings(OUT UE::Net::FRootObjectSettings& OutSettings) const
{
	Super::ConfigureObjectSettings(OutSettings);
	
	OutSettings.FactoryName = UFlecsNetEntityProxyNetFactory::GetFactoryName();
}

bool UFlecsNetEntityProxy::CanAcceptNetEntity(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot&) const
{
	return !bContainsEntity || NetworkId == InNetworkId;
}

void UFlecsNetEntityProxy::PublishNetEntity(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot)
{
	solid_checkf(CanAcceptNetEntity(InNetworkId, InSnapshot),
		TEXT("Cannot publish network ID '%s' to occupied Flecs entity proxy '%s'"),
		*InNetworkId.ToString(), *GetName());

	bContainsEntity = true;
	NetworkId = InNetworkId;
	Snapshot = InSnapshot;

	MARK_PROPERTY_DIRTY_FROM_NAME(UFlecsNetEntityProxy, NetworkId, this);
	MARK_PROPERTY_DIRTY_FROM_NAME(UFlecsNetEntityProxy, Snapshot, this);
}

void UFlecsNetEntityProxy::RemoveNetEntity(const FFlecsNetworkId& InNetworkId)
{
	solid_checkf(bContainsEntity && NetworkId == InNetworkId,
		TEXT("Cannot remove network ID '%s' from Flecs entity proxy '%s'"),
		*InNetworkId.ToString(), *GetName());

	bContainsEntity = false;
}

bool UFlecsNetEntityProxy::IsEmpty() const
{
	return !bContainsEntity;
}

void UFlecsNetEntityProxy::OnRep_NetworkId()
{
	bContainsEntity = NetworkId.IsValid();
	ReceiveEntityUpdate(NetworkId, Snapshot);
}

void UFlecsNetEntityProxy::OnRep_Snapshot()
{
	ReceiveEntityUpdate(NetworkId, Snapshot);
}

void UFlecsNetEntityProxy::HandleReplicationDetached()
{
	bContainsEntity = false;
	ReceiveEntityRemoval(NetworkId, Snapshot.StateRevision);
}
