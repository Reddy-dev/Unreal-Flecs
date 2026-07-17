// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsIrisReplicationFilter.h"

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
	}
}

void UFlecsIrisReplicationFilter::OnInit(const FNetObjectFilterInitParams& Params)
{
	ReplicationSystem = Params.ReplicationSystem;
}

void UFlecsIrisReplicationFilter::OnDeinit()
{
	Pages.Reset();
	ReplicationSystem.Reset();
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
		const bool bAllowed = Subsystem->IsRouteRelevant(Page->GetRouteDescriptor(), Params.ConnectionId, View);
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
