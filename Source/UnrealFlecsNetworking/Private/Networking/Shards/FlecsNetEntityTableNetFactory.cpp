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

	const TSolidNotNull<UFlecsNetEntityTable*> Table = CastChecked<UFlecsNetEntityTable>(Result.Instance);
	UWorld* World = UE::Flecs::Replication::GetReplicationBridgeWorld(Bridge);

	if UNLIKELY_IF(!World)
	{
		Result.Instance = nullptr;
		Result.Template = nullptr;
		Result.FailureDiagnosticMessage = TEXT("The receiving replication bridge does not have a valid UWorld");
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

void UFlecsNetEntityTableNetFactory::FillRootObjectReplicationParams(
	const UE::Net::FRootObjectReplicationParamsContext& Context, UE::Net::FRootObjectReplicationParams& OutParams)
{
	Super::FillRootObjectReplicationParams(Context, OutParams);
}
