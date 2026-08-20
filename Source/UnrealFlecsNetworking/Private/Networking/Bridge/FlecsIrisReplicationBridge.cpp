// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Bridge/FlecsIrisReplicationBridge.h"

#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Net/UnrealNetwork.h"

#include "Networking/Bridge/FlecsIrisReplicationBridgeNetFactory.h"
#include "Networking/Profiles/FlecsProfileRelationshipTypes.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"
#include "Networking/Shards/FlecsNetEntityProxy.h"
#include "Networking/Shards/FlecsNetShardBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsIrisReplicationBridge)

FFlecsReplicationShardPoolKey::FFlecsReplicationShardPoolKey(const FFlecsEntityView& InProfile,
	const FFlecsReplicationShardSelection& InSelection): ShardClass(InSelection.ShardClass.Get())
	                                                     , ShardGroupKey(InSelection.ShardGroupKey)
{
	solid_check(InProfile.IsValid());
	
	if (const FFlecsNetProfileNameTarget* ObjectNameTarget 
		= InProfile.TryGetPairSecond<FFlecsObjectPrioritizerRelationship, FFlecsNetProfileNameTarget>())
	{
		ObjectPrioritizerName = ObjectNameTarget->Name;
	}
	else
	{
		ObjectPrioritizerName = NAME_None;
	}
	
	if (const FFlecsNetProfileNameTarget* FilterNameTarget 
		= InProfile.TryGetPairSecond<FFlecsNetFilterRelationship, FFlecsNetProfileNameTarget>())
	{
		FilterName = FilterNameTarget->Name;
	}
	else
	{
		FilterName = NAME_None;
	}
}

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
	for (TPair<FFlecsReplicationShardPoolKey, TArray<TObjectPtr<UFlecsNetShardBase>>>& Pair : ShardPools)
	{
		for (UFlecsNetShardBase* Shard : Pair.Value)
		{
			if LIKELY_IF(Shard)
			{
				Shard->DeinitializeShard();
				Shard->SetOwningNetworkWorldSubsystem(nullptr);
			}
		}
	}
	
	ShardMap.Reset();
	ShardPools.Reset();

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
	solid_checkf(EntityHandle.IsValid(), TEXT("Cannot publish a Flecs entity without a valid entity handle"));
	
	const TSolidNotNull<UFlecsNetShardBase*> Shard = ResolveShard(EntityHandle, InNetworkId, InSnapshot);
	Shard->PublishNetEntity(InNetworkId, InSnapshot);
}

void UFlecsIrisReplicationBridge::StopReplicatingEntity(const FFlecsEntityHandle& InEntityHandle)
{
	if (!HasAuthority())
	{
		return;
	}

	if (const FFlecsReplicationShardPlacement* Placement = ShardMap.Find(InEntityHandle))
	{
		if (UFlecsNetShardBase* Shard = Placement->Shard.Get())
		{
			Shard->RemoveNetEntity(Placement->NetworkId);
			ReleaseShardIfEmpty(Shard, Placement->Profile, Placement->Selection);
		}

		ShardMap.Remove(InEntityHandle);
	}
}

UFlecsNetShardBase* UFlecsIrisReplicationBridge::ResolveShard(const FFlecsEntityHandle& InEntityHandle,
	const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot)
{
	const TSolidNotNull<UFlecsNetworkWorldSubsystem*> NetworkSubsystem = GetNetworkWorldSubsystem();

	FFlecsEntityView Profile;
	const bool bResolvedProfile = NetworkSubsystem->ResolveReplicationProfile(InEntityHandle, Profile);
	solid_cassumef(bResolvedProfile,
		TEXT("Cannot resolve a replication profile for entity '%s'"), *InEntityHandle.ToString());

	FFlecsReplicationShardSelection Selection;
	const bool bSelectedShard = NetworkSubsystem->SelectReplicationShard(InEntityHandle, InNetworkId, Profile, Selection);
	solid_cassumef(bSelectedShard,
		TEXT("Cannot select a replication shard for entity '%s' with network ID '%s'"), *InEntityHandle.ToString(), *InNetworkId.ToString());

	if (FFlecsReplicationShardPlacement* Placement = ShardMap.Find(InEntityHandle))
	{
		if (UFlecsNetShardBase* ExistingShard = Placement->Shard.Get())
		{
			if (Placement->Profile == Profile && Placement->Selection == Selection
				&& ExistingShard->CanAcceptNetEntity(InNetworkId, InSnapshot))
			{
				Placement->NetworkId = InNetworkId;
				return ExistingShard;
			}
		}

		UFlecsNetShardBase* DestinationShard 
			= FindOrCreateShard(InNetworkId, InSnapshot, Profile, Selection);
		
		if UNLIKELY_IF(!ensure(DestinationShard))
		{
			return nullptr;
		}

		// Publish the full baseline before removing the old physical target.
		DestinationShard->PublishNetEntity(InNetworkId, InSnapshot);

		UFlecsNetShardBase* SourceShard = Placement->Shard.Get();
		
		if (SourceShard && SourceShard != DestinationShard)
		{
			SourceShard->RemoveNetEntity(Placement->NetworkId);
			ReleaseShardIfEmpty(SourceShard, Placement->Profile, Placement->Selection);
		}

		Placement->Shard = DestinationShard;
		Placement->Profile = Profile;
		Placement->Selection = Selection;
		Placement->NetworkId = InNetworkId;
		Placement->TargetGeneration++;
		Placement->PlacementGeneration++;
		return DestinationShard;
	}
	
	UFlecsNetShardBase* Shard 
		= FindOrCreateShard(InNetworkId, InSnapshot, Profile, Selection);
	
	if UNLIKELY_IF(!ensure(Shard))
	{
		return nullptr;
	}

	FFlecsReplicationShardPlacement& Placement = ShardMap.Add(InEntityHandle);
	Placement.Shard = Shard;
	Placement.Profile = Profile;
	Placement.Selection = Selection;
	Placement.NetworkId = InNetworkId;
	Placement.TargetGeneration = 1;
	Placement.PlacementGeneration = 1;
	
	return Shard;
}

UFlecsNetShardBase* UFlecsIrisReplicationBridge::CreateNewShard(const FFlecsNetworkId& InNetworkId,
	const FFlecsEntityReplicationSnapshot& InSnapshot,
	const FFlecsEntityView& InProfile, const FFlecsReplicationShardSelection& InSelection)
{
	solid_checkf(InNetworkId.IsValid(), TEXT("Cannot create a Flecs replication shard without a valid network ID"));

	const TSolidNotNull<UFlecsNetShardBase*> Shard = NewObject<UFlecsNetShardBase>(this, InSelection.ShardClass);

	Shard->SetOwningNetworkWorldSubsystem(GetNetworkWorldSubsystem());
	Shard->InitializeShard(InProfile);
	Shard->StartShardReplication();

	if UNLIKELY_IF(!Shard->CanAcceptNetEntity(InNetworkId, InSnapshot))
	{
		UE_LOG(LogFlecsCore, Error,
			TEXT("New Flecs replication shard '%s' rejected network ID '%s'"),
			*Shard->GetName(), *InNetworkId.ToString());
		
		Shard->DeinitializeShard();
		Shard->SetOwningNetworkWorldSubsystem(nullptr);
		Shard->MarkAsGarbage();
		return nullptr;
	}

	return Shard;
}

UFlecsNetShardBase* UFlecsIrisReplicationBridge::FindOrCreateShard(const FFlecsNetworkId& InNetworkId,
	const FFlecsEntityReplicationSnapshot& InSnapshot, const FFlecsEntityView& InProfile,
	const FFlecsReplicationShardSelection& InSelection)
{
	const FFlecsReplicationShardPoolKey PoolKey(InProfile, InSelection);
	if (const TArray<TObjectPtr<UFlecsNetShardBase>>* Shards = ShardPools.Find(PoolKey))
	{
		for (UFlecsNetShardBase* Shard : *Shards)
		{
			if (Shard && Shard->CanAcceptNetEntity(InNetworkId, InSnapshot))
			{
				return Shard;
			}
		}
	}

	UFlecsNetShardBase* NewShard = CreateNewShard(InNetworkId, InSnapshot, InProfile, InSelection);
	if UNLIKELY_IF(!NewShard)
	{
		return nullptr;
	}

	ShardPools.FindOrAdd(PoolKey).Add(NewShard);
	return NewShard;
}

void UFlecsIrisReplicationBridge::ReleaseShardIfEmpty(UFlecsNetShardBase* InShard,
	const FFlecsEntityView& InProfile, const FFlecsReplicationShardSelection& InSelection)
{
	if (!InShard || !InShard->IsEmpty())
	{
		return;
	}

	const FFlecsReplicationShardPoolKey PoolKey(InProfile, InSelection);
	TArray<TObjectPtr<UFlecsNetShardBase>>* Shards = ShardPools.Find(PoolKey);
	if (!Shards)
	{
		return;
	}

	Shards->RemoveSingle(InShard);
	if (Shards->IsEmpty())
	{
		ShardPools.Remove(PoolKey);
	}

	InShard->DeinitializeShard();
	InShard->SetOwningNetworkWorldSubsystem(nullptr);
}
