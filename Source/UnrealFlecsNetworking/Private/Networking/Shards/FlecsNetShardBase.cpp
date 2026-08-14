// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Shards/FlecsNetShardBase.h"

#include "Engine/World.h"
#include "Engine/NetDriver.h"
#include "Iris/ReplicationSystem/ObjectReplicationBridge.h"
#include "Iris/ReplicationSystem/Filtering/NetObjectFilter.h"
#include "Iris/ReplicationSystem/Prioritization/NetObjectPrioritizer.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Iris/ReplicationSystem/ReplicationSystem.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

#include "Networking/FlecsReplicationProfile.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetShardBase)

UWorld* UFlecsNetShardBase::GetWorld() const
{
	return OwningWorld.IsValid() ? OwningWorld.Get() : Super::GetWorld();
}

void UFlecsNetShardBase::InitializeShard()
{
	if (RootObjectAdapter.IsInitialized())
	{
		return;
	}

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
	OutSettings.bIsAlwaysRelevant = true;
	OutSettings.bIsNotRouted = false;
}

bool UFlecsNetShardBase::ApplyReplicationProfile(const FFlecsReplicationProfile& InProfile)
{
	const UWorld* World = GetWorld();
	const UNetDriver* NetDriver = World ? World->GetNetDriver() : nullptr;
	UReplicationSystem* ReplicationSystem = NetDriver ? NetDriver->GetReplicationSystem() : nullptr;
	
	if (!ReplicationSystem)
	{
		return true;
	}

	UObjectReplicationBridge* ReplicationBridge = ReplicationSystem->GetReplicationBridge();
	if UNLIKELY_IF(!ReplicationBridge)
	{
		UE_LOG(LogFlecsCore, Error,
			TEXT("Cannot apply a Flecs replication profile to '%s' without an Iris object bridge"), *GetName());
		return false;
	}

	const UE::Net::FNetRefHandle NetRefHandle = ReplicationBridge->GetReplicatedRefHandle(this);
	if UNLIKELY_IF(!NetRefHandle.IsValid())
	{
		UE_LOG(LogFlecsCore, Error,
			TEXT("Cannot apply a Flecs replication profile to '%s' without a valid Iris handle"), *GetName());
		return false;
	}

	if (!InProfile.FilterName.IsNone())
	{
		const UE::Net::FNetObjectFilterHandle FilterHandle = ReplicationSystem->GetFilterHandle(InProfile.FilterName);
		
		if UNLIKELY_IF(FilterHandle == UE::Net::InvalidNetObjectFilterHandle)
		{
			UE_LOG(LogFlecsCore, Error,
				TEXT("Cannot apply unknown Iris filter '%s' to Flecs shard '%s'"),
				*InProfile.FilterName.ToString(), *GetName());
			return false;
		}

		if UNLIKELY_IF(!ReplicationSystem->SetFilter(NetRefHandle, FilterHandle))
		{
			UE_LOG(LogFlecsCore, Error,
				TEXT("Iris rejected filter '%s' for Flecs shard '%s'"),
				*InProfile.FilterName.ToString(), *GetName());
			return false;
		}
	}

	if (!InProfile.ObjectPrioritizerName.IsNone())
	{
		const UE::Net::FNetObjectPrioritizerHandle PrioritizerHandle = ReplicationSystem->GetPrioritizerHandle(InProfile.ObjectPrioritizerName);
		if UNLIKELY_IF(PrioritizerHandle == UE::Net::InvalidNetObjectPrioritizerHandle)
		{
			UE_LOG(LogFlecsCore, Error,
				TEXT("Cannot apply unknown Iris prioritizer '%s' to Flecs shard '%s'"),
				*InProfile.ObjectPrioritizerName.ToString(), *GetName());
			return false;
		}

		if UNLIKELY_IF(!ReplicationSystem->SetPrioritizer(NetRefHandle, PrioritizerHandle))
		{
			UE_LOG(LogFlecsCore, Error,
				TEXT("Iris rejected prioritizer '%s' for Flecs shard '%s'"),
				*InProfile.ObjectPrioritizerName.ToString(), *GetName());
			return false;
		}
	}

	return true;
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

void UFlecsNetShardBase::ResolveOwningNetworkWorldSubsystem()
{
	if (OwningNetworkWorldSubsystem.IsValid())
	{
		StopOwningNetworkWorldSubsystemRetry();
		return;
	}

	UWorld* World = GetWorld();
	if (!World || World->bIsTearingDown)
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
	if (!NetworkSubsystem)
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

void UFlecsNetShardBase::ReceiveEntityUpdate(const FFlecsNetworkId& InNetworkId,
	const FFlecsEntityReplicationSnapshot& InSnapshot)
{
	if (!InNetworkId.IsValid() || !InSnapshot.LayoutId.IsValid())
	{
		return;
	}

	ResolveOwningNetworkWorldSubsystem();
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
	if (!InNetworkId.IsValid())
	{
		return;
	}

	ResolveOwningNetworkWorldSubsystem();
	UFlecsNetworkWorldSubsystem* NetworkSubsystem = GetOwningNetworkWorldSubsystem();
	
	if (!NetworkSubsystem)
	{
		PendingReplicationUpdateQueue.EnqueueRemoval(InNetworkId, InStateRevision);
		return;
	}

	NetworkSubsystem->RemoveReceivedNetworkEntity(InNetworkId, InStateRevision);
}
