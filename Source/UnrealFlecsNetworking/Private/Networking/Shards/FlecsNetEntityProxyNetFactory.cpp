// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Shards/FlecsNetEntityProxyNetFactory.h"

#include "Engine/World.h"
#include "Iris/ReplicationSystem/ObjectReplicationBridge.h"

#include "Networking/Bridge/FlecsIrisReplicationWorldResolver.h"
#include "Networking/Shards/FlecsNetEntityProxy.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetEntityProxyNetFactory)

FName UFlecsNetEntityProxyNetFactory::GetFactoryName()
{
	static const FName FactoryName(TEXT("FlecsNetEntityProxyNetFactory"));
	return FactoryName;
}

UNetObjectFactory::FInstantiateResult UFlecsNetEntityProxyNetFactory::InstantiateReplicatedObjectFromHeader(
	const FInstantiateContext& Context,
	const UE::Net::FNetObjectCreationHeader* Header)
{
	FInstantiateResult Result = Super::InstantiateReplicatedObjectFromHeader(Context, Header);
	if UNLIKELY_IF(!Result.Instance)
	{
		return Result;
	}

	UFlecsNetEntityProxy* Proxy = Cast<UFlecsNetEntityProxy>(Result.Instance);
	UWorld* World = UE::Flecs::Replication::GetReplicationBridgeWorld(Bridge);

	if UNLIKELY_IF(!Proxy || !World)
	{
		Result.Instance = nullptr;
		Result.Template = nullptr;
		Result.FailureDiagnosticMessage = !Proxy
			? TEXT("Instantiated an object that is not a UFlecsNetEntityProxy")
			: TEXT("The receiving replication bridge does not have a valid UWorld");
		return Result;
	}

	Proxy->SetOwningWorld(World);
	return Result;
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

TOptional<UNetObjectFactory::FWorldInfoData> UFlecsNetEntityProxyNetFactory::GetWorldInfo(
	const FWorldInfoContext& Context) const
{
	const UFlecsNetEntityProxy* Proxy = Cast<UFlecsNetEntityProxy>(Context.Instance);
	
	if UNLIKELY_IF(!Proxy)
	{
		UE_LOG(LogNet, Warning, 
			TEXT("UFlecsNetEntityProxyNetFactory::GetWorldInfo: GetWorldInfo called on a non-UFlecsNetEntityProxy instance"));
		return TOptional<FWorldInfoData>();
	}
	
	TOptional<UNetObjectFactory::FWorldInfoData> WorldInfoData = Proxy->GetWorldInfoData();
	return WorldInfoData;
}
