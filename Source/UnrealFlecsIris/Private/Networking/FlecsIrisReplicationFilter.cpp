// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsIrisReplicationFilter.h"

#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Iris/ReplicationSystem/ReplicationSystem.h"
#include "Net/Iris/ReplicationSystem/EngineReplicationBridge.h"
#include "Networking/FlecsIrisReplicationShard.h"
#include "Networking/FlecsNetworkWorldSubsystem.h"
#include "Networking/FlecsNetworkingStats.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsIrisReplicationFilter)

FName UFlecsIrisReplicationFilter::GetFilterName()
{
	static const FName Name(TEXT("FlecsReplicationInterest"));
	return Name;
}

void UFlecsIrisReplicationFilter::RegisterPage(const UE::Net::FNetRefHandle Handle,
	UFlecsIrisReplicationShard* Page)
{
	if (!Handle.IsValid() || !Page)
	{
		return;
	}
	const UE::Net::FInternalNetRefIndex ObjectIndex = GetObjectIndex(Handle);
	if (UE::Net::IsValidInternalNetRefIndex(ObjectIndex))
	{
		Pages.Add(ObjectIndex, Page);
		NetworkSubsystem = Page->GetWorld()
			? Page->GetWorld()->GetSubsystem<UFlecsNetworkWorldSubsystem>() : nullptr;
	}
}

void UFlecsIrisReplicationFilter::OnInit(const FNetObjectFilterInitParams& Params)
{
	ReplicationSystem = Params.ReplicationSystem;
	const UEngineReplicationBridge* EngineBridge = Params.ReplicationSystem
		? Cast<UEngineReplicationBridge>(Params.ReplicationSystem->GetReplicationBridge()) : nullptr;
	const UNetDriver* NetDriver = EngineBridge ? EngineBridge->GetNetDriver() : nullptr;
	NetworkSubsystem = NetDriver && NetDriver->GetWorld()
		? NetDriver->GetWorld()->GetSubsystem<UFlecsNetworkWorldSubsystem>() : nullptr;
}

void UFlecsIrisReplicationFilter::OnDeinit()
{
	if (UFlecsNetworkWorldSubsystem* Subsystem = NetworkSubsystem.Get())
	{
		for (const uint32 ConnectionId : Connections)
		{
			Subsystem->ClearConnectionInterestContext(ConnectionId);
		}
	}
	Connections.Reset();
	Pages.Reset();
	NetworkSubsystem.Reset();
	ReplicationSystem.Reset();
}

void UFlecsIrisReplicationFilter::AddConnection(const uint32 ConnectionId)
{
	Super::AddConnection(ConnectionId);
	Connections.Add(ConnectionId);
}

void UFlecsIrisReplicationFilter::RemoveConnection(const uint32 ConnectionId)
{
	if (UFlecsNetworkWorldSubsystem* Subsystem = NetworkSubsystem.Get())
	{
		Subsystem->ClearConnectionInterestContext(ConnectionId);
	}
	Connections.Remove(ConnectionId);
	Super::RemoveConnection(ConnectionId);
}

void UFlecsIrisReplicationFilter::OnMaxInternalNetRefIndexIncreased(
	UE::Net::FInternalNetRefIndex)
{
}

bool UFlecsIrisReplicationFilter::AddObject(UE::Net::FInternalNetRefIndex,
	FNetObjectFilterAddObjectParams&)
{
	return true;
}

void UFlecsIrisReplicationFilter::RemoveObject(const UE::Net::FInternalNetRefIndex ObjectIndex,
	const FNetObjectFilteringInfo&)
{
	Pages.Remove(ObjectIndex);
}

void UFlecsIrisReplicationFilter::Filter(FNetObjectFilteringParams& Params)
{
	Params.OutAllowedObjects.ClearAllBits();
	GetFilteredObjects().ForAllSetBits([this, &Params](const uint32 ObjectIndex)
	{
		const TWeakObjectPtr<UFlecsIrisReplicationShard>* PagePtr = Pages.Find(ObjectIndex);
		UFlecsIrisReplicationShard* Page = PagePtr ? PagePtr->Get() : nullptr;
		if (!Page)
		{
			// Registration follows StartReplication in the same server tick. Do not
			// destroy a page on a connection during that short hand-off window.
			Params.OutAllowedObjects.SetBit(ObjectIndex);
			return;
		}
		UFlecsNetworkWorldSubsystem* Subsystem = Page->GetWorld()
			? Page->GetWorld()->GetSubsystem<UFlecsNetworkWorldSubsystem>() : nullptr;
		if (!Subsystem)
		{
			return;
		}
		FFlecsReplicationConnectionView View;
		for (const UE::Net::FReplicationView::FView& IrisView : Params.View.Views)
		{
			View.Positions.Add(IrisView.Pos);
			View.Directions.Add(IrisView.Dir);
		}
		const FFlecsReplicationRouteDescriptor& Route = Page->GetRouteDescriptor();
		const bool bAllowed = Subsystem->IsInterestBindingRelevant(
			Route.Interest, Route.LogicalKey.Name, Params.ConnectionId, View);
		Params.OutAllowedObjects.SetBitValue(ObjectIndex, bAllowed);
		if (bAllowed)
		{
			INC_DWORD_STAT(STAT_FlecsReplicationFilterAllowed);
		}
		else
		{
			INC_DWORD_STAT(STAT_FlecsReplicationFilterDenied);
		}
	});
}
