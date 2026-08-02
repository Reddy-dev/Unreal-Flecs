// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Shards/FlecsNetEntityProxyNetFactory.h"

#include "Engine/World.h"
#include "Iris/ReplicationSystem/ObjectReplicationBridge.h"

#include "Networking/Shards/FlecsNetEntityProxy.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetEntityProxyNetFactory)

FName UFlecsNetEntityProxyNetFactory::GetFactoryName()
{
	static const FName FactoryName(TEXT("FlecsNetEntityProxyNetFactory"));
	return FactoryName;
}

void UFlecsNetEntityProxyNetFactory::PostInstantiation(const FPostInstantiationContext& Context)
{
	Super::PostInstantiation(Context);

	UFlecsNetEntityProxy* Proxy = Cast<UFlecsNetEntityProxy>(Context.Instance);
	const UWorld* World = Bridge ? Bridge->GetWorld() : nullptr;
	UFlecsNetworkWorldSubsystem* NetworkSubsystem = World ? World->GetSubsystem<UFlecsNetworkWorldSubsystem>() : nullptr;

	if UNLIKELY_IF(!Proxy || !NetworkSubsystem)
	{
		UE_LOG(LogFlecsCore, Error,
			TEXT("Could not bind a received Flecs entity proxy to its network world"));
		return;
	}

	Proxy->SetOwningNetworkWorldSubsystem(NetworkSubsystem);
}

void UFlecsNetEntityProxyNetFactory::DetachedFromReplication(const FDetachContext& Context,
	const TOptional<FSubObjectDetachContext>& SubObjectContext)
{
	if (UFlecsNetEntityProxy* Proxy = Cast<UFlecsNetEntityProxy>(Context.DetachedInstance))
	{
		Proxy->HandleReplicationDetached();

		if (Context.Reason != UE::Net::EDetachReason::TornOff)
		{
			Proxy->SetOwningNetworkWorldSubsystem(nullptr);
		}
	}

	Super::DetachedFromReplication(Context, SubObjectContext);
}
