// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsIrisReplicationFilter.h"

#include "Engine/NetConnection.h"
#include "GameFramework/PlayerController.h"
#include "Iris/ReplicationSystem/ReplicationSystem.h"

#include "Networking/FlecsIrisReplicationShard.h"
#include "Networking/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsIrisReplicationFilter)

FFlecsReplicationConnectionId GetFlecsReplicationConnectionId(const UNetConnection* Connection)
{
	return Connection
		? FFlecsReplicationConnectionId(Connection->GetConnectionHandle().GetParentConnectionId())
		: FFlecsReplicationConnectionId{};
}

FFlecsReplicationConnectionId GetFlecsReplicationConnectionId(const APlayerController* Controller)
{
	return Controller ? GetFlecsReplicationConnectionId(Controller->GetNetConnection())
		: FFlecsReplicationConnectionId{};
}

FName UFlecsIrisReplicationFilter::GetFilterName()
{
	static const FName Name(TEXT("FlecsReplicationInterest"));
	return Name;
}

void UFlecsIrisReplicationFilter::RegisterPage(const UE::Net::FNetRefHandle Handle, UFlecsIrisReplicationShard* Page)
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

void UFlecsIrisReplicationFilter::RemoveConnection(const uint32 ConnectionId)
{
	Super::RemoveConnection(ConnectionId);
	
	Connections.Remove(ConnectionId);
	
	if (UFlecsNetworkWorldSubsystem* Subsystem = NetworkSubsystem.Get())
	{
		Subsystem->ClearConnectionInterestContext(FFlecsReplicationConnectionId(ConnectionId));
	}
}

void UFlecsIrisReplicationFilter::AddConnection(const uint32 ConnectionId)
{
	Super::AddConnection(ConnectionId);
	
	Connections.Add(ConnectionId);
}

void UFlecsIrisReplicationFilter::OnInit(const FNetObjectFilterInitParams& Params)
{
	ReplicationSystem = Params.ReplicationSystem;
}

void UFlecsIrisReplicationFilter::OnDeinit()
{
	if (UFlecsNetworkWorldSubsystem* Subsystem = NetworkSubsystem.Get())
	{
		for (const uint32 ConnectionId : Connections)
		{
			Subsystem->ClearConnectionInterestContext(FFlecsReplicationConnectionId(ConnectionId));
		}
	}
	
	Connections.Reset();
	Pages.Reset();
	NetworkSubsystem.Reset();
	ReplicationSystem.Reset();
}

void UFlecsIrisReplicationFilter::OnMaxInternalNetRefIndexIncreased(UE::Net::FInternalNetRefIndex)
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
			View.FieldOfViewRadians.Add(IrisView.FoVRadians);
		}
		
		const bool bAllowed = Subsystem->IsRouteRelevant(Page->GetRouteDescriptor(),
			FFlecsReplicationConnectionId(Params.ConnectionId), View);
		
		Params.OutAllowedObjects.SetBitValue(ObjectIndex, bAllowed);
	});
}
