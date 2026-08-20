// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Shards/FlecsNetShardBase.h"

#include "Engine/World.h"
#include "Engine/NetDriver.h"
#include "Iris/ReplicationSystem/ObjectReplicationBridge.h"
#include "Iris/ReplicationSystem/Filtering/NetObjectFilter.h"
#include "Iris/ReplicationSystem/Prioritization/NetObjectPrioritizer.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Iris/ReplicationSystem/ReplicationSystem.h"

#include "Networking/Profiles/FlecsNetAlwaysRelevantTag.h"
#include "Networking/Profiles/FlecsProfileRelationshipTypes.h"
#include "Networking/Profiles/FlecsReplicationProfile.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetShardBase)

UWorld* UFlecsNetShardBase::GetWorld() const
{
	return OwningWorld.IsValid() ? OwningWorld.Get() : Super::GetWorld();
}

void UFlecsNetShardBase::InitializeShard(const FFlecsEntityView& InReplicationProfile)
{
	if (RootObjectAdapter.IsInitialized())
	{
		return;
	}
	
	ReplicationProfile = InReplicationProfile;

	UE::Net::FRootObjectSettings Settings;
	ConfigureObjectSettings(Settings);
	
	RootObjectAdapter.InitAdapter(this);
	RootObjectAdapter.Configure(Settings);
}

void UFlecsNetShardBase::DeinitializeShard()
{
	StopShardReplication();
	
	StopOwningNetworkWorldSubsystemRetry();
	RootObjectAdapter.DeinitAdapter();
}

void UFlecsNetShardBase::StartShardReplication()
{
	if (!RootObjectAdapter.IsInitialized() || RootObjectAdapter.IsReplicating())
	{
		return;
	}

	const UWorld* World = GetWorld();
	if UNLIKELY_IF(!World || !World->PersistentLevel)
	{
		return;
	}

	RootObjectAdapter.StartReplication(World->PersistentLevel);
	ApplyReplicationProfile();
}

void UFlecsNetShardBase::StopShardReplication()
{
	if (RootObjectAdapter.IsReplicating())
	{
		RootObjectAdapter.StopReplication();
	}
}

void UFlecsNetShardBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
}

void UFlecsNetShardBase::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Fragments,
	UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Fragments, RegistrationFlags);
}

void UFlecsNetShardBase::FillRootObjectReplicationParams(const UE::Net::FRootObjectReplicationParamsContext& Context,
	UE::Net::FRootObjectReplicationParams& OutParams) const
{
	RootObjectAdapter.FillRootObjectReplicationParams(Context, OutParams);
}

void UFlecsNetShardBase::ConfigureObjectSettings(OUT UE::Net::FRootObjectSettings& OutSettings) const
{
	const bool bHasAlwaysRelevantTag = GetReplicationProfile().Has<FFlecsNetAlwaysRelevantTag>();
	
	OutSettings.bIsAlwaysRelevant = true;
	OutSettings.bIsNotRouted = false;
}

void UFlecsNetShardBase::ApplyReplicationProfile() const
{
	const TSolidNotNull<const UWorld*> World = GetWorld();
	const TSolidNotNull<const UNetDriver*> NetDriver = World->GetNetDriver();
	const TSolidNotNull<UReplicationSystem*> ReplicationSystem = NetDriver->GetReplicationSystem();

	const TSolidNotNull<const UObjectReplicationBridge*> ReplicationBridge = ReplicationSystem->GetReplicationBridge();

	const UE::Net::FNetRefHandle NetRefHandle = ReplicationBridge->GetReplicatedRefHandle(this);
	solid_checkf(NetRefHandle.IsValid(), TEXT("Flecs shard '%s' is not registered with the Iris replication system"), *GetName());
	
	if (const FFlecsNetProfileNameTarget* FilterName 
		= GetReplicationProfile().TryGetPairSecond<FFlecsNetFilterRelationship, FFlecsNetProfileNameTarget>())
	{
		const UE::Net::FNetObjectFilterHandle FilterHandle = ReplicationSystem->GetFilterHandle(FilterName->Name);
		
		solid_cassumef(FilterHandle != UE::Net::InvalidNetObjectFilterHandle, 
			TEXT("Flecs shard '%s' is not registered with the Iris replication system"), *GetName());
		
		const bool bFilterSet = ReplicationSystem->SetFilter(NetRefHandle, FilterHandle);
		solid_cassumef(bFilterSet, TEXT("Iris rejected filter '%s' for Flecs shard '%s'"),
			*FilterName->Name.ToString(), *GetName());
	}

	if (const FFlecsNetProfileNameTarget* PrioritizerName 
		= GetReplicationProfile().TryGetPairSecond<FFlecsObjectPrioritizerRelationship, FFlecsNetProfileNameTarget>())
	{
		const UE::Net::FNetObjectPrioritizerHandle PrioritizerHandle
			= ReplicationSystem->GetPrioritizerHandle(PrioritizerName->Name);
		
		solid_cassumef(PrioritizerHandle != UE::Net::InvalidNetObjectPrioritizerHandle, 
			TEXT("Flecs shard '%s' is not registered with the Iris replication system"), *GetName());
		
		const bool bPrioritizerSet = ReplicationSystem->SetPrioritizer(NetRefHandle, PrioritizerHandle);
		solid_cassumef(bPrioritizerSet, TEXT("Iris rejected prioritizer '%s' for Flecs shard '%s'"),
			*PrioritizerName->Name.ToString(), *GetName());
	}
}

void UFlecsNetShardBase::SetOwningNetworkWorldSubsystem(UFlecsNetworkWorldSubsystem* InOwningNetworkWorldSubsystem)
{
	OwningNetworkWorldSubsystem = InOwningNetworkWorldSubsystem;

	if (!InOwningNetworkWorldSubsystem)
	{
		StopOwningNetworkWorldSubsystemRetry();
		return;
	}

	OwningWorld = InOwningNetworkWorldSubsystem->GetWorld();
	StopOwningNetworkWorldSubsystemRetry();
	FlushPendingReplicationUpdates();
}

TOptional<UNetObjectFactory::FWorldInfoData> UFlecsNetShardBase::GetWorldInfoData() const
{
	return TOptional<UNetObjectFactory::FWorldInfoData>();
}

void UFlecsNetShardBase::SetOwningWorld(const TSolidNotNull<UWorld*> InOwningWorld)
{
	OwningWorld = InOwningWorld;
	ResolveOwningNetworkWorldSubsystem();
}

UFlecsNetworkWorldSubsystem* UFlecsNetShardBase::GetOwningNetworkWorldSubsystem() const
{
	return OwningNetworkWorldSubsystem.Get();
}

const FFlecsEntityView& UFlecsNetShardBase::GetReplicationProfile() const
{
	return ReplicationProfile;
}

void UFlecsNetShardBase::ResolveOwningNetworkWorldSubsystem()
{
	if (OwningNetworkWorldSubsystem.IsValid())
	{
		StopOwningNetworkWorldSubsystemRetry();
		return;
	}

	const UWorld* World = GetWorld();
	
	if UNLIKELY_IF(!World || World->bIsTearingDown)
	{
		return;
	}

	if (UFlecsNetworkWorldSubsystem* NetworkSubsystem = World->GetSubsystem<UFlecsNetworkWorldSubsystem>())
	{
		SetOwningNetworkWorldSubsystem(NetworkSubsystem);
		return;
	}

	StartOwningNetworkWorldSubsystemRetry();
}

void UFlecsNetShardBase::HandleWorldPreActorTick(UWorld* InWorld, ELevelTick, float)
{
	if (InWorld != GetWorld())
	{
		return;
	}

	ResolveOwningNetworkWorldSubsystem();
}

void UFlecsNetShardBase::StartOwningNetworkWorldSubsystemRetry()
{
	if (!WorldPreActorTickHandle.IsValid())
	{
		WorldPreActorTickHandle = FWorldDelegates::OnWorldPreActorTick.AddUObject(
			this, &UFlecsNetShardBase::HandleWorldPreActorTick);
	}
}

void UFlecsNetShardBase::StopOwningNetworkWorldSubsystemRetry()
{
	if (WorldPreActorTickHandle.IsValid())
	{
		FWorldDelegates::OnWorldPreActorTick.Remove(WorldPreActorTickHandle);
		
		WorldPreActorTickHandle.Reset();
	}
}

void UFlecsNetShardBase::FlushPendingReplicationUpdates()
{
	UFlecsNetworkWorldSubsystem* NetworkSubsystem = GetOwningNetworkWorldSubsystem();
	if UNLIKELY_IF(!NetworkSubsystem)
	{
		return;
	}

	const TArray<FFlecsReplicationQueuedUpdate> Updates = PendingReplicationUpdateQueue.Drain();
	for (const FFlecsReplicationQueuedUpdate& Update : Updates)
	{
		if (Update.bRemove)
		{
			NetworkSubsystem->RemoveReceivedNetworkEntity(Update.NetworkId, Update.StateRevision);
		}
		else
		{
			NetworkSubsystem->ReceiveNetworkEntitySnapshot(Update.NetworkId, Update.Snapshot);
		}
	}
}

void UFlecsNetShardBase::ReceiveEntityUpdate(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot)
{
	if (!InNetworkId.IsValid() || !InSnapshot.LayoutId.IsValid())
	{
		return;
	}

	ResolveOwningNetworkWorldSubsystem();
	
	// this may be Null
	UFlecsNetworkWorldSubsystem* NetworkSubsystem = GetOwningNetworkWorldSubsystem();
	if UNLIKELY_IF(!NetworkSubsystem)
	{
		PendingReplicationUpdateQueue.EnqueueSnapshot(InNetworkId, InSnapshot);
		return;
	}

	NetworkSubsystem->ReceiveNetworkEntitySnapshot(InNetworkId, InSnapshot);
}

void UFlecsNetShardBase::ReceiveEntityRemoval(const FFlecsNetworkId& InNetworkId, const uint32 InStateRevision)
{
	solid_checkf(InNetworkId.IsValid(), TEXT("Invalid network id for entity removal"));

	ResolveOwningNetworkWorldSubsystem();
	UFlecsNetworkWorldSubsystem* NetworkSubsystem = GetOwningNetworkWorldSubsystem();
	
	if (!NetworkSubsystem)
	{
		PendingReplicationUpdateQueue.EnqueueRemoval(InNetworkId, InStateRevision);
		return;
	}

	NetworkSubsystem->RemoveReceivedNetworkEntity(InNetworkId, InStateRevision);
}
