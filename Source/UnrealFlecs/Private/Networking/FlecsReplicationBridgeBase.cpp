// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsReplicationBridgeBase.h"

#include "Engine/World.h"

#include "Networking/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationBridgeBase)

UFlecsReplicationBridgeBase::UFlecsReplicationBridgeBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlecsReplicationBridgeBase::~UFlecsReplicationBridgeBase()
{
}

void UFlecsReplicationBridgeBase::PublishEntityLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition)
{
	OnEntityLayoutPublished(InLayoutDefinition);
}

void UFlecsReplicationBridgeBase::ReceiveEntityLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition)
{
	GetNetworkWorldSubsystem()->GetLayoutRegistry().AddRemoteDefinition(InLayoutDefinition);
	
	OnEntityLayoutPublished(InLayoutDefinition);
}

void UFlecsReplicationBridgeBase::ReceiveNetEntity(const FFlecsNetworkId& InNetworkId,
	const FFlecsEntityReplicationSnapshot& InSnapshot)
{
	GetNetworkWorldSubsystem()->ReceiveNetworkEntitySnapshot(InNetworkId, InSnapshot);
}

void UFlecsReplicationBridgeBase::HandleProtocolError(const FString& InErrorMessage)
{
	UE_LOG(LogFlecsCore, Error, TEXT("Protocol error: %s"), *InErrorMessage);
}

TSolidNotNull<UFlecsNetworkWorldSubsystem*> UFlecsReplicationBridgeBase::GetNetworkWorldSubsystem() const
{
	return GetWorld()->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
}

bool UFlecsReplicationBridgeBase::HasAuthority() const
{
	return GetNetworkWorldSubsystem()->HasAuthority();
}
