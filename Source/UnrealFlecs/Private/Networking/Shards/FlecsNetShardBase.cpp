// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Shards/FlecsNetShardBase.h"

#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetShardBase)

void UFlecsNetShardBase::InitializeShard()
{
	UE::Net::FRootObjectSettings Settings;
	ConfigureObjectSettings(Settings);
	
	RootObjectAdapter.Configure(Settings);
	RootObjectAdapter.InitAdapter(this);
}

void UFlecsNetShardBase::DeinitializeShard()
{
	RootObjectAdapter.DeinitAdapter();
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
}
