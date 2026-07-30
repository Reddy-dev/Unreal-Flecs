// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Bridge/FlecsIrisReplicationBridge.h"

#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Net/UnrealNetwork.h"

#include "Networking/Bridge/FlecsIrisReplicationBridgeNetFactory.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"
#include "Networking/Router/FlecsReplicationRouterBase.h"
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
	LifetimeParams.bIsPushBased = false;
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsIrisReplicationBridge, ReplicatedLayouts, LifetimeParams);
}

void UFlecsIrisReplicationBridge::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Fragments,
	UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(
		this, Fragments, RegistrationFlags);
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

	if (!TryStartReplication())
	{
		WorldPreActorTickHandle = FWorldDelegates::OnWorldPreActorTick.AddUObject(
			this, &UFlecsIrisReplicationBridge::HandleWorldPreActorTick);
	}
}

void UFlecsIrisReplicationBridge::DeinitializeBridge()
{
	if (WorldPreActorTickHandle.IsValid())
	{
		FWorldDelegates::OnWorldPreActorTick.Remove(WorldPreActorTickHandle);
		WorldPreActorTickHandle.Reset();
	}

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

bool UFlecsIrisReplicationBridge::TryStartReplication()
{
	if (RootObjectAdapter.IsReplicating())
	{
		return true;
	}

	if (!HasAuthority())
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

void UFlecsIrisReplicationBridge::HandleWorldPreActorTick(UWorld* InWorld, ELevelTick InTickType, float InDeltaSeconds)
{
	if (InWorld != GetWorld())
	{
		return;
	}

	if (TryStartReplication())
	{
		FWorldDelegates::OnWorldPreActorTick.Remove(WorldPreActorTickHandle);
		WorldPreActorTickHandle.Reset();
	}
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

void UFlecsIrisReplicationBridge::ReceiveLayout(
	const FFlecsReplicationLayoutDefinition& InLayoutDefinition)
{
	ReceiveEntityLayout(InLayoutDefinition);
}

void UFlecsIrisReplicationBridge::PublishNetEntity(
	const FFlecsNetRouteId& InRouteId,
	const FFlecsNetworkId& InNetworkId,
	const FFlecsEntityReplicationSnapshot& InSnapshot)
{
	
}

/*UFlecsNetShardBase* UFlecsIrisReplicationBridge::ResolveShard(const FFlecsNetRouteId& InRouteId,
	const FFlecsEntityHandle& InEntityHandle)
{
	if UNLIKELY_IF(!InRouteId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid route id passed to ResolveShard"));
		return nullptr;
	}
	
	EFlecsReplicationRoutedShardType ShardType = GetNetworkWorldSubsystem()->GetReplicationRouter()
		->GetRoutedShardType(InEntityHandle, InRouteId);
	
	// @TODO: switch to a switch
	if (ShardType == EFlecsReplicationRoutedShardType::Proxy)
	{
		
	}
	else if (ShardType == EFlecsReplicationRoutedShardType::Paged)
	{
		
	}
	
	checkf(false, TEXT("Unsupported shard type %d"), static_cast<int32>(ShardType));
	return nullptr;
}*/
