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
	if UNLIKELY_IF(!FlecsBridge)
	{
		UE_LOG(LogFlecsCore, Error,
			TEXT("Received object is not a Flecs Iris replication bridge"));
		return;
	}

	PendingReplicationBridge = FlecsBridge;
	ResolvePendingReplicationBridge();
}

void UFlecsIrisReplicationBridgeNetFactory::DetachedFromReplication(
	const FDetachContext& Context,
	const TOptional<FSubObjectDetachContext>& SubObjectContext)
{
	UFlecsIrisReplicationBridge* FlecsBridge = Cast<UFlecsIrisReplicationBridge>(Context.DetachedInstance);
	if (PendingReplicationBridge.Get() == FlecsBridge)
	{
		PendingReplicationBridge = nullptr;
		StopReplicationBridgeRetry();
	}

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

void UFlecsIrisReplicationBridgeNetFactory::OnDeinit()
{
	PendingReplicationBridge = nullptr;
	StopReplicationBridgeRetry();
	Super::OnDeinit();
}

void UFlecsIrisReplicationBridgeNetFactory::ResolvePendingReplicationBridge()
{
	UFlecsIrisReplicationBridge* FlecsBridge = PendingReplicationBridge.Get();
	if (!FlecsBridge)
	{
		StopReplicationBridgeRetry();
		return;
	}

	UWorld* World = Bridge ? Bridge->GetWorld() : nullptr;
	if (!World)
	{
		if (!WorldPreActorTickHandle.IsValid())
		{
			WorldPreActorTickHandle = FWorldDelegates::OnWorldPreActorTick.AddUObject(
				this, &UFlecsIrisReplicationBridgeNetFactory::HandleWorldPreActorTick);
		}
		return;
	}

	if (World->bIsTearingDown)
	{
		return;
	}

	if (UFlecsNetworkWorldSubsystem* NetworkSubsystem = World->GetSubsystem<UFlecsNetworkWorldSubsystem>())
	{
		NetworkSubsystem->BindReplicationBridge(FlecsBridge);
		PendingReplicationBridge = nullptr;
		StopReplicationBridgeRetry();
		return;
	}

	if (!WorldPreActorTickHandle.IsValid())
	{
		WorldPreActorTickHandle = FWorldDelegates::OnWorldPreActorTick.AddUObject(
			this, &UFlecsIrisReplicationBridgeNetFactory::HandleWorldPreActorTick);
	}
}

void UFlecsIrisReplicationBridgeNetFactory::HandleWorldPreActorTick(UWorld* InWorld, ELevelTick, float)
{
	if (InWorld != (Bridge ? Bridge->GetWorld() : nullptr))
	{
		return;
	}

	ResolvePendingReplicationBridge();
}

void UFlecsIrisReplicationBridgeNetFactory::StopReplicationBridgeRetry()
{
	if (WorldPreActorTickHandle.IsValid())
	{
		FWorldDelegates::OnWorldPreActorTick.Remove(WorldPreActorTickHandle);
		WorldPreActorTickHandle.Reset();
	}
}
