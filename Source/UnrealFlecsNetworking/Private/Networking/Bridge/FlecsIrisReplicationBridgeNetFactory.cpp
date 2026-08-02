// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Bridge/FlecsIrisReplicationBridgeNetFactory.h"

#include "Engine/World.h"
#include "Iris/ReplicationSystem/ObjectReplicationBridge.h"

#include "Networking/Bridge/FlecsIrisReplicationBridge.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsIrisReplicationBridgeNetFactory)

FName UFlecsIrisReplicationBridgeNetFactory::GetFactoryName()
{
	static const FName FactoryName(TEXT("FlecsIrisReplicationBridgeNetFactory"));
	return FactoryName;
}

void UFlecsIrisReplicationBridgeNetFactory::PostInstantiation(const FPostInstantiationContext& Context)
{
	Super::PostInstantiation(Context);

	UFlecsIrisReplicationBridge* FlecsBridge = Cast<UFlecsIrisReplicationBridge>(Context.Instance);
	UWorld* World = Bridge ? Bridge->GetWorld() : nullptr;
	
	UFlecsNetworkWorldSubsystem* NetworkSubsystem =
		World ? World->GetSubsystem<UFlecsNetworkWorldSubsystem>() : nullptr;

	if UNLIKELY_IF(!FlecsBridge || !NetworkSubsystem)
	{
		UE_LOG(LogFlecsCore, Error,
			TEXT("Could not bind a received Flecs Iris replication bridge to its network world"));
		return;
	}

	NetworkSubsystem->BindReplicationBridge(FlecsBridge);
}

void UFlecsIrisReplicationBridgeNetFactory::DetachedFromReplication(
	const FDetachContext& Context,
	const TOptional<FSubObjectDetachContext>& SubObjectContext)
{
	UFlecsIrisReplicationBridge* FlecsBridge = Cast<UFlecsIrisReplicationBridge>(Context.DetachedInstance);
	UWorld* World = Bridge ? Bridge->GetWorld() : nullptr;
	UFlecsNetworkWorldSubsystem* NetworkSubsystem = World ? World->GetSubsystem<UFlecsNetworkWorldSubsystem>() : nullptr;

	if (Context.Reason != UE::Net::EDetachReason::TornOff
		&& FlecsBridge
		&& NetworkSubsystem
		&& !NetworkSubsystem->HasAuthority())
	{
		NetworkSubsystem->UnbindReplicationBridge(FlecsBridge);
	}

	Super::DetachedFromReplication(Context, SubObjectContext);
}
