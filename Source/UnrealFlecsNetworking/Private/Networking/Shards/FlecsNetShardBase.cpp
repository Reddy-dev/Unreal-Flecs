// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Shards/FlecsNetShardBase.h"

#include "Engine/World.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetShardBase)

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

void UFlecsNetShardBase::SetOwningNetworkWorldSubsystem(UFlecsNetworkWorldSubsystem* InOwningNetworkWorldSubsystem)
{
	OwningNetworkWorldSubsystem = InOwningNetworkWorldSubsystem;
}

UFlecsNetworkWorldSubsystem* UFlecsNetShardBase::GetOwningNetworkWorldSubsystem() const
{
	return OwningNetworkWorldSubsystem.Get();
}

void UFlecsNetShardBase::ReceiveEntityUpdate(const FFlecsNetworkId& InNetworkId,
	const FFlecsEntityReplicationSnapshot& InSnapshot)
{
	if (!InNetworkId.IsValid() || !InSnapshot.LayoutId.IsValid())
	{
		return;
	}

	UFlecsNetworkWorldSubsystem* NetworkSubsystem = GetOwningNetworkWorldSubsystem();
	if UNLIKELY_IF(!NetworkSubsystem)
	{
		UE_LOG(LogFlecsCore, Error,
			TEXT("Received Flecs entity update for '%s' without an owning network world"),
			*InNetworkId.ToString());
		return;
	}

	NetworkSubsystem->ReceiveNetworkEntitySnapshot(InNetworkId, InSnapshot);
}
