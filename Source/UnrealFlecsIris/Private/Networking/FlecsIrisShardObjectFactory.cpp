// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsIrisShardObjectFactory.h"

#include "Engine/NetDriver.h"
#include "Net/Iris/ReplicationSystem/EngineReplicationBridge.h"
#include "Networking/FlecsIrisReplicationShard.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsIrisShardObjectFactory)

FName UFlecsIrisShardObjectFactory::GetFactoryName()
{
	static const FName Name(TEXT("FlecsIrisShardFactory"));
	return Name;
}

void UFlecsIrisShardObjectFactory::PostInit(const FPostInitContext& Context)
{
	Super::PostInit(Context);
	if (UFlecsIrisReplicationShard* Shard = Cast<UFlecsIrisReplicationShard>(Context.Instance))
	{
		const UEngineReplicationBridge* EngineBridge = Cast<UEngineReplicationBridge>(Bridge);
		UNetDriver* NetDriver = EngineBridge ? EngineBridge->GetNetDriver() : nullptr;
		Shard->BindClient(NetDriver ? NetDriver->GetWorld() : nullptr);
		Shard->EnqueueAllReceived();
	}
}

void UFlecsIrisShardObjectFactory::DetachedFromReplication(const FDetachContext& Context,
	const TOptional<FSubObjectDetachContext>& SubObjectContext)
{
	if (UFlecsIrisReplicationShard* Shard = Cast<UFlecsIrisReplicationShard>(Context.DetachedInstance))
	{
		Shard->DetachedFromReplication();
	}
	Super::DetachedFromReplication(Context, SubObjectContext);
}
