// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Shards/FlecsNetEntityTableNetFactory.h"

#include "Engine/World.h"
#include "Iris/ReplicationSystem/ObjectReplicationBridge.h"

#include "Networking/Bridge/FlecsIrisReplicationWorldResolver.h"
#include "Networking/Shards/FlecsNetEntityTable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetEntityTableNetFactory)

FName UFlecsNetEntityTableNetFactory::GetFactoryName()
{
	static const FName FactoryName(TEXT("FlecsNetEntityTableNetFactory"));
	return FactoryName;
}

UNetObjectFactory::FInstantiateResult UFlecsNetEntityTableNetFactory::InstantiateReplicatedObjectFromHeader(
	const FInstantiateContext& Context,
	const UE::Net::FNetObjectCreationHeader* Header)
{
	FInstantiateResult Result = Super::InstantiateReplicatedObjectFromHeader(Context, Header);
	if UNLIKELY_IF(!Result.Instance)
	{
		return Result;
	}

	UFlecsNetEntityTable* Table = Cast<UFlecsNetEntityTable>(Result.Instance);
	UWorld* World = UE::Flecs::Replication::GetReplicationBridgeWorld(Bridge);

	if UNLIKELY_IF(!Table || !World)
	{
		Result.Instance = nullptr;
		Result.Template = nullptr;
		Result.FailureDiagnosticMessage = !Table
			? TEXT("Instantiated an object that is not a UFlecsNetEntityTable")
			: TEXT("The receiving replication bridge does not have a valid UWorld");
		return Result;
	}

	Table->SetOwningWorld(World);
	return Result;
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
