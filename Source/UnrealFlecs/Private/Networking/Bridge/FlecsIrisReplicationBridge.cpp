// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Bridge/FlecsIrisReplicationBridge.h"

#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Net/UnrealNetwork.h"

#include "Networking/Bridge/FlecsIrisReplicationBridgeNetFactory.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"
#include "Networking/Shards/FlecsNetEntityProxy.h"
#include "Networking/Shards/FlecsNetShardBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsIrisReplicationBridge)

UFlecsIrisReplicationBridge::UFlecsIrisReplicationBridge(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ReplicatedLayouts.SetOwner(this);
}

void UFlecsIrisReplicationBridge::PostInitProperties()
{
	Super::PostInitProperties();
	
	ReplicatedLayouts.SetOwner(this);
}

void UFlecsIrisReplicationBridge::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams LifetimeParams;
	LifetimeParams.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsIrisReplicationBridge, ReplicatedLayouts, LifetimeParams);
}

void UFlecsIrisReplicationBridge::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Fragments,
	UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Fragments, RegistrationFlags);
}

void UFlecsIrisReplicationBridge::FillRootObjectReplicationParams(
	const UE::Net::FRootObjectReplicationParamsContext& Context,
	UE::Net::FRootObjectReplicationParams& OutParams) const
{
	RootObjectAdapter.FillRootObjectReplicationParams(Context, OutParams);
}

void UFlecsIrisReplicationBridge::InitializeBridge()
{
	ReplicatedLayouts.SetOwner(this);

	if (!HasAuthority())
	{
		for (const FFlecsLayoutReplicatorItem& Item : ReplicatedLayouts.Items)
		{
			ReceiveLayout(Item.LayoutDefinition);
		}

		return;
	}

	UE::Net::FRootObjectSettings Settings;
	Settings.bIsAlwaysRelevant = true;
	Settings.bIsNotRouted = false;
	Settings.FactoryName = UFlecsIrisReplicationBridgeNetFactory::GetFactoryName();

	RootObjectAdapter.InitAdapter(this);
	RootObjectAdapter.Configure(Settings);

	const UWorld* World = GetWorld();
	if (World && World->GetNetMode() != NM_Standalone)
	{
		const UNetDriver* NetDriver = World->GetNetDriver();
		if (NetDriver && NetDriver->GetReplicationSystem())
		{
			RootObjectAdapter.StartReplication(World->PersistentLevel);
		}
	}
}

void UFlecsIrisReplicationBridge::DeinitializeBridge()
{
	for (TPair<FFlecsEntityView, TObjectPtr<UFlecsNetShardBase>>& Pair : ShardMap)
	{
		if (UFlecsNetShardBase* Shard = Pair.Value.Get())
		{
			Shard->DeinitializeShard();
			Shard->SetOwningNetworkWorldSubsystem(nullptr);
		}
	}
	ShardMap.Reset();

	if (RootObjectAdapter.IsReplicating())
	{
		RootObjectAdapter.StopReplication();
	}

	if (RootObjectAdapter.IsInitialized())
	{
		RootObjectAdapter.DeinitAdapter();
	}

	SetNetworkWorldSubsystem(nullptr);
}

void UFlecsIrisReplicationBridge::PublishEntityLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition)
{
	if UNLIKELY_IF(!HasAuthority())
	{
		UE_LOG(LogFlecsCore, Error, TEXT("Cannot publish a Flecs layout without authority"));
		return;
	}

	ReplicatedLayouts.AddLayout(InLayoutDefinition);
}

void UFlecsIrisReplicationBridge::ReceiveLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition)
{
	ReceiveEntityLayout(InLayoutDefinition);
}

void UFlecsIrisReplicationBridge::PublishNetEntity(const FFlecsEntityHandle& EntityHandle, const FFlecsNetworkId& InNetworkId,
	const FFlecsEntityReplicationSnapshot& InSnapshot)
{
	UFlecsNetShardBase* Shard = ResolveShard(EntityHandle, InNetworkId);
	if UNLIKELY_IF(!Shard)
	{
		return;
	}

	Shard->PublishNetEntity(InNetworkId, InSnapshot);
}

void UFlecsIrisReplicationBridge::StopReplicatingEntity(const FFlecsEntityHandle& InEntityHandle)
{
	if (!HasAuthority())
	{
		return;
	}

	if (TObjectPtr<UFlecsNetShardBase>* ShardPtr = ShardMap.Find(InEntityHandle))
	{
		if (UFlecsNetShardBase* Shard = ShardPtr->Get())
		{
			Shard->DeinitializeShard();
			Shard->SetOwningNetworkWorldSubsystem(nullptr);
		}

		ShardMap.Remove(InEntityHandle);
	}
}

UFlecsNetShardBase* UFlecsIrisReplicationBridge::ResolveShard(const FFlecsEntityHandle& InEntityHandle, const FFlecsNetworkId& InNetworkId)
{
	if (TObjectPtr<UFlecsNetShardBase>* ShardPtr = ShardMap.Find(InEntityHandle))
	{
		return ShardPtr->Get();
	}
	
	return CreateNewShard(InEntityHandle, InNetworkId);
}

UFlecsNetShardBase* UFlecsIrisReplicationBridge::CreateNewShard(const FFlecsEntityHandle& InEntityHandle,
	const FFlecsNetworkId& InNetworkId)
{
	if UNLIKELY_IF(!InNetworkId.IsValid())
	{
		UE_LOG(LogFlecsCore, Error, TEXT("Cannot create a Flecs entity proxy without a valid network ID"));
		return nullptr;
	}

	UFlecsNetShardBase* Shard = NewObject<UFlecsNetEntityProxy>(this);
	if UNLIKELY_IF(!Shard)
	{
		return nullptr;
	}

	Shard->SetOwningNetworkWorldSubsystem(GetNetworkWorldSubsystem());
	Shard->InitializeShard();
	ShardMap.Add(InEntityHandle, Shard);

	Shard->StartShardReplication();

	return Shard;
}
