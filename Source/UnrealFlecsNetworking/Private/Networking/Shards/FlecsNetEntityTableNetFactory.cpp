// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Shards/FlecsNetEntityTableNetFactory.h"

#include "Engine/World.h"
#include "Iris/ReplicationSystem/ObjectReplicationBridge.h"

#include "Networking/Shards/FlecsNetEntityTable.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetEntityTableNetFactory)

FName UFlecsNetEntityTableNetFactory::GetFactoryName()
{
	static const FName FactoryName(TEXT("FlecsNetEntityTableNetFactory"));
	return FactoryName;
}

void UFlecsNetEntityTableNetFactory::PostInstantiation(const FPostInstantiationContext& Context)
{
	Super::PostInstantiation(Context);

	UFlecsNetEntityTable* Table = Cast<UFlecsNetEntityTable>(Context.Instance);
	const UWorld* World = Bridge ? Bridge->GetWorld() : nullptr;
	UFlecsNetworkWorldSubsystem* NetworkSubsystem = World ? World->GetSubsystem<UFlecsNetworkWorldSubsystem>() : nullptr;

	if UNLIKELY_IF(!Table || !NetworkSubsystem)
	{
		UE_LOG(LogFlecsCore, Error,
			TEXT("Could not bind a received Flecs entity table to its network world"));
		return;
	}

	Table->SetOwningNetworkWorldSubsystem(NetworkSubsystem);
}

void UFlecsNetEntityTableNetFactory::DetachedFromReplication(const FDetachContext& Context,
	const TOptional<FSubObjectDetachContext>& SubObjectContext)
{
	if (UFlecsNetEntityTable* Table = Cast<UFlecsNetEntityTable>(Context.DetachedInstance))
	{
		Table->HandleReplicationDetached();

		if (Context.Reason != UE::Net::EDetachReason::TornOff)
		{
			Table->SetOwningNetworkWorldSubsystem(nullptr);
		}
	}

	Super::DetachedFromReplication(Context, SubObjectContext);
}
