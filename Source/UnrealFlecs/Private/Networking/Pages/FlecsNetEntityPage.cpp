// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Pages/FlecsNetEntityPage.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetEntityPage)

void UFlecsNetEntityPage::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams LifetimeParams;
	LifetimeParams.bIsPushBased = false;
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsNetEntityPage, EntityPage, LifetimeParams);
}

void UFlecsNetEntityPage::OnRep_EntityPage()
{
}
