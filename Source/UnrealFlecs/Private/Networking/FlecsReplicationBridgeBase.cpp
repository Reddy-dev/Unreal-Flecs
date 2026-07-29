// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsReplicationBridgeBase.h"

#include "Engine/World.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"

#include "Networking/FlecsNetworkWorldSubsystem.h"
#include "Networking/Shards/FlecsNetShardBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationBridgeBase)

UFlecsReplicationBridgeBase::UFlecsReplicationBridgeBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlecsReplicationBridgeBase::~UFlecsReplicationBridgeBase()
{
}

void UFlecsReplicationBridgeBase::ReceiveEntityLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition)
{
	const TValueOrError<void, FString> Result =
		GetNetworkWorldSubsystem()->GetLayoutRegistry().AddRemoteDefinition(InLayoutDefinition);

	if UNLIKELY_IF(Result.HasError())
	{
		HandleProtocolError(Result.GetError());
		return;
	}
	
	GetNetworkWorldSubsystem()->OnEntityLayoutReceived(InLayoutDefinition);
}

void UFlecsReplicationBridgeBase::ReceiveEntityLayoutRemoval(const FFlecsReplicationLayoutId& InLayoutId)
{
	(void)GetNetworkWorldSubsystem()->GetLayoutRegistry().RemoveRemoteDefinition(InLayoutId);
	GetNetworkWorldSubsystem()->OnEntityLayoutRemoved(InLayoutId);
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
