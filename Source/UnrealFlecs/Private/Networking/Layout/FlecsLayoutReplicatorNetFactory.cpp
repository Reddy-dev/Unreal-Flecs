// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Layout/FlecsLayoutReplicatorNetFactory.h"

#include "Iris/ReplicationSystem/ObjectReplicationBridge.h"

#include "Networking/FlecsIrisReplicationBridge.h"
#include "Networking/FlecsNetworkWorldSubsystem.h"
#include "Networking/Layout/FlecsLayoutReplicator.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsLayoutReplicatorNetFactory)

FName UFlecsLayoutReplicatorNetFactory::GetFactoryName()
{
	static const FName FactoryName(TEXT("FlecsLayoutReplicatorNetFactory"));
	return FactoryName;
}

void UFlecsLayoutReplicatorNetFactory::PostInstantiation(
	const FPostInstantiationContext& Context)
{
	Super::PostInstantiation(Context);

	UFlecsLayoutReplicator* Replicator = Cast<UFlecsLayoutReplicator>(Context.Instance);
	UWorld* World = Bridge ? Bridge->GetWorld() : nullptr;
	UFlecsNetworkWorldSubsystem* NetworkSubsystem =
		World ? World->GetSubsystem<UFlecsNetworkWorldSubsystem>() : nullptr;

	if UNLIKELY_IF(!Replicator || !NetworkSubsystem)
	{
		UE_LOG(LogFlecsCore, Error,
			TEXT("Could not bind a received Flecs layout replicator to its network world"));
		return;
	}

	UFlecsIrisReplicationBridge* FlecsBridge =
		NetworkSubsystem->GetReplicationBridge<UFlecsIrisReplicationBridge>();
	FlecsBridge->BindLayoutReplicator(Replicator);
}
