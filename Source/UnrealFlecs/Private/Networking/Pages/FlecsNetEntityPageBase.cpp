// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Pages/FlecsNetEntityPageBase.h"

#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetEntityPageBase)

void UFlecsNetEntityPageBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams LifetimeParams;
	LifetimeParams.bIsPushBased = false;
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsNetEntityPageBase, EntityPage, LifetimeParams);
}

void UFlecsNetEntityPageBase::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Fragments,
	UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Fragments, RegistrationFlags);
}

void UFlecsNetEntityPageBase::OnRep_EntityPage()
{
}
