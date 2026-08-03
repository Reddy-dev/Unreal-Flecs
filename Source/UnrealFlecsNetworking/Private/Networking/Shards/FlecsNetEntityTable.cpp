// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Shards/FlecsNetEntityTable.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetEntityTable)

void UFlecsNetEntityTable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams LifetimeParams;
	LifetimeParams.bIsPushBased = false;
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsNetEntityTable, EntityTable, LifetimeParams);
}

void UFlecsNetEntityTable::OnRep_EntityTable()
{
}
