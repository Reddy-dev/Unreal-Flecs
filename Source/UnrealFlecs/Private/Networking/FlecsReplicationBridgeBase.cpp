// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsReplicationBridgeBase.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationBridgeBase)

AFlecsReplicationBridgeBase::AFlecsReplicationBridgeBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetReplicates(true);
	
}

void AFlecsReplicationBridgeBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AFlecsReplicationBridgeBase::BeginPlay()
{
	Super::BeginPlay();
	
}
