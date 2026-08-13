// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Bridge/FlecsReplicationBridgeBase.h"

#include "Engine/World.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"

#include "Worlds/FlecsWorld.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"
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
	const TSolidNotNull<const UFlecsNetworkWorldSubsystem*> LocalNetworkWorldSubsystem = GetNetworkWorldSubsystem();
	
	// Not guaranteed to be valid
	const UFlecsWorld* FlecsWorld = UFlecsWorld::GetDefaultWorld(LocalNetworkWorldSubsystem);
	
	const TValueOrError<bool, FString> Result =
		GetNetworkWorldSubsystem()->GetLayoutRegistry().AddRemoteDefinition(InLayoutDefinition, FlecsWorld);

	if UNLIKELY_IF(Result.HasError())
	{
		HandleProtocolError(Result.GetError());
		return;
	}
	
	GetNetworkWorldSubsystem()->OnEntityLayoutReceived(InLayoutDefinition);
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

void UFlecsReplicationBridgeBase::SetNetworkWorldSubsystem(
	UFlecsNetworkWorldSubsystem* InNetworkWorldSubsystem)
{
	NetworkWorldSubsystem = InNetworkWorldSubsystem;
}

TSolidNotNull<UFlecsNetworkWorldSubsystem*> UFlecsReplicationBridgeBase::GetNetworkWorldSubsystem() const
{
	if LIKELY_IF(NetworkWorldSubsystem.IsValid())
	{
		return NetworkWorldSubsystem.Get();
	}

	return GetWorld()->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
}

bool UFlecsReplicationBridgeBase::HasAuthority() const
{
	return GetNetworkWorldSubsystem()->HasAuthority();
}
