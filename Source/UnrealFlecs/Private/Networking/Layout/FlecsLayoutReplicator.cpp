// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Layout/FlecsLayoutReplicator.h"

#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Net/UnrealNetwork.h"

#include "Networking/FlecsReplicationBridgeBase.h"
#include "Networking/Layout/FlecsLayoutReplicatorNetFactory.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsLayoutReplicator)

UFlecsLayoutReplicator::UFlecsLayoutReplicator()
{
	ReplicatedLayouts.SetOwner(this);
}

void UFlecsLayoutReplicator::PostInitProperties()
{
	Super::PostInitProperties();
	ReplicatedLayouts.SetOwner(this);
}

void UFlecsLayoutReplicator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams LifetimeParams;
	LifetimeParams.bIsPushBased = false;
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsLayoutReplicator, ReplicatedLayouts, LifetimeParams);
}

void UFlecsLayoutReplicator::RegisterReplicationFragments(
	UE::Net::FFragmentRegistrationContext& Fragments,
	UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(
		this, Fragments, RegistrationFlags);
}

void UFlecsLayoutReplicator::FillRootObjectReplicationParams(
	const UE::Net::FRootObjectReplicationParamsContext& Context,
	UE::Net::FRootObjectReplicationParams& OutParams) const
{
	RootObjectAdapter.FillRootObjectReplicationParams(Context, OutParams);
}

void UFlecsLayoutReplicator::InitializeReplicator(const TSolidNotNull<UFlecsReplicationBridgeBase*> InReplicationBridge)
{
	check(IsValid(InReplicationBridge));

	BindReplicationBridge(InReplicationBridge);

	UE::Net::FRootObjectSettings Settings;
	Settings.bIsAlwaysRelevant = true;
	Settings.bIsNotRouted = false;
	Settings.FactoryName = UFlecsLayoutReplicatorNetFactory::GetFactoryName();

	RootObjectAdapter.InitAdapter(this);
	RootObjectAdapter.Configure(Settings);

	TryStartReplication();
}

void UFlecsLayoutReplicator::DeinitializeReplicator()
{
	if (RootObjectAdapter.IsReplicating())
	{
		RootObjectAdapter.StopReplication();
	}

	if (RootObjectAdapter.IsInitialized())
	{
		RootObjectAdapter.DeinitAdapter();
	}

	ReplicationBridge.Reset();
}

bool UFlecsLayoutReplicator::TryStartReplication()
{
	if (RootObjectAdapter.IsReplicating())
	{
		return true;
	}

	if (!ReplicationBridge.IsValid() || !ReplicationBridge->HasAuthority())
	{
		return true;
	}

	const UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Standalone)
	{
		return true;
	}

	const UNetDriver* NetDriver = World->GetNetDriver();
	if (!NetDriver || !NetDriver->GetReplicationSystem())
	{
		return false;
	}

	RootObjectAdapter.StartReplication(World->PersistentLevel);
	return RootObjectAdapter.IsReplicating();
}

void UFlecsLayoutReplicator::BindReplicationBridge(const TSolidNotNull<UFlecsReplicationBridgeBase*> InReplicationBridge)
{
	ReplicationBridge = InReplicationBridge;
	ReplicatedLayouts.SetOwner(this);

	for (const FFlecsLayoutReplicatorItem& Item : ReplicatedLayouts.Items)
	{
		ReceiveLayout(Item.LayoutDefinition);
	}
}

void UFlecsLayoutReplicator::PublishLayout(
	const FFlecsReplicationLayoutDefinition& InLayoutDefinition)
{
	if UNLIKELY_IF(!ReplicationBridge.IsValid() || !ReplicationBridge->HasAuthority())
	{
		UE_LOG(LogFlecsCore, Error, TEXT("Cannot publish a Flecs layout without an authority replication bridge"));
		return;
	}

	ReplicatedLayouts.AddLayout(InLayoutDefinition);
}

void UFlecsLayoutReplicator::ReceiveLayout(
	const FFlecsReplicationLayoutDefinition& InLayoutDefinition)
{
	if (ReplicationBridge.IsValid())
	{
		ReplicationBridge->ReceiveEntityLayout(InLayoutDefinition);
	}
}
