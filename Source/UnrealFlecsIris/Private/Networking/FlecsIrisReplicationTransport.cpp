// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsIrisReplicationTransport.h"

#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Iris/ReplicationSystem/Filtering/NetObjectFilterDefinitions.h"
#include "Iris/ReplicationSystem/ReplicationSystem.h"

#include "Networking/FlecsIrisReplicationFilter.h"
#include "Networking/FlecsIrisReplicationShard.h"
#include "Networking/FlecsNetworkingModuleSettings.h"
#include "Networking/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsIrisReplicationTransport)

bool UFlecsIrisReplicationTransport::InitializeTransport(UFlecsNetworkWorldSubsystem* InSubsystem)
{
	const bool bInitialized = Super::InitializeTransport(InSubsystem) && InSubsystem && InSubsystem->GetWorld()
		&& InSubsystem->GetWorld()->GetNetMode() != NM_Standalone;
	
	if (bInitialized)
	{
		EnsureReplicationFilter();
	}
	
	return bInitialized;
}

void UFlecsIrisReplicationTransport::ShutdownTransport()
{
	for (UFlecsIrisReplicationShard* Page : Pages)
	{
		if (Page)
		{
			Page->StopReplication();
		}
	}
	
	Pages.Reset();
	EntityPages.Reset();
	Layouts.Reset();
	MaterializedEntities.Reset();
	PendingRetirementTicks.Reset();
	if (bCreatedRuntimeFilter)
	{
		const UFlecsNetworkWorldSubsystem* Subsystem = GetNetworkSubsystem();
		const UNetDriver* Driver = Subsystem && Subsystem->GetWorld() ? Subsystem->GetWorld()->GetNetDriver() : nullptr;
		if (UReplicationSystem* ReplicationSystem = Driver ? Driver->GetReplicationSystem() : nullptr)
		{
			ReplicationSystem->DestroyFilter(UFlecsIrisReplicationFilter::GetFilterName());
		}
	}
	bCreatedRuntimeFilter = false;
	
	Super::ShutdownTransport();
}

void UFlecsIrisReplicationTransport::TickTransport()
{
	EnsureReplicationFilter();
	bool bAllStarted = true;
	
	for (UFlecsIrisReplicationShard* Page : Pages)
	{
		if (Page)
		{
			bAllStarted &= Page->TryStartReplication();
		}
	}

	TArray<UFlecsIrisReplicationShard*> Retired;
	
	for (TPair<TObjectPtr<UFlecsIrisReplicationShard>, uint8>& Pair : PendingRetirementTicks)
	{
		if (!Pair.Key || !Pair.Key->IsEmpty())
		{
			Retired.Add(Pair.Key);
			continue;
		}
		
		if (Pair.Value++ >= 1)
		{
			Pair.Key->StopReplication();
			Pages.Remove(Pair.Key);
			Retired.Add(Pair.Key);
		}
	}
	for (UFlecsIrisReplicationShard* Page : Retired)
	{
		PendingRetirementTicks.Remove(Page);
	}
	
	if (!Pages.IsEmpty() && !bAllStarted && ++StartAttempts > 120 && !bWarnedReplicationSystemUnavailable)
	{
		bWarnedReplicationSystemUnavailable = true;
		UE_LOG(LogFlecsCore, Warning,
			TEXT("UnrealFlecs Iris shards could not attach to a compatible ReplicationSystem. Enable Iris on the active NetDriver; Flecs simulation remains active without network transport."));
	}
}

void UFlecsIrisReplicationTransport::EnsureReplicationFilter()
{
	UFlecsNetworkWorldSubsystem* Subsystem = GetNetworkSubsystem();
	UNetDriver* Driver = Subsystem && Subsystem->GetWorld() ? Subsystem->GetWorld()->GetNetDriver() : nullptr;
	UReplicationSystem* ReplicationSystem = Driver ? Driver->GetReplicationSystem() : nullptr;
	if (!ReplicationSystem || ReplicationSystem->GetFilter(UFlecsIrisReplicationFilter::GetFilterName()))
	{
		return;
	}

	FNetObjectFilterDefinition Definition;
	Definition.FilterName = UFlecsIrisReplicationFilter::GetFilterName();
	Definition.ClassName = FName(*UFlecsIrisReplicationFilter::StaticClass()->GetPathName());
	bCreatedRuntimeFilter = ReplicationSystem->CreateFilter(Definition).IsValid();
}

void UFlecsIrisReplicationTransport::PublishLayout(const FFlecsReplicationRouteDescriptor& Route,
	const FFlecsReplicationLayoutDefinition& Layout)
{
	Layouts.Add(Layout.LayoutId, Layout);
	for (UFlecsIrisReplicationShard* Page : Pages)
	{
		if (Page && Page->GetRouteDescriptor() == Route)
		{
			Page->UpsertLayout(Layout);
		}
	}
}

void UFlecsIrisReplicationTransport::PublishEntity(const FFlecsReplicationRouteDescriptor& Route,
	const FFlecsReplicatedEntityUpdate& Update)
{
	FFlecsReplicatedEntityUpdate FullState = MaterializeUpdate(Update);
	
	UFlecsIrisReplicationShard* CurrentPage = EntityPages.FindRef(Update.NetworkId);
	
	if (CurrentPage && CurrentPage->GetRouteDescriptor() == Route && CurrentPage->CanFitUpdate(Update))
	{
		PublishToPage(*CurrentPage, Update);
		MaterializedEntities.Add(Update.NetworkId, MoveTemp(FullState));
		return;
	}

	UFlecsIrisReplicationShard* Destination = FindPageForUpdate(Route, FullState, CurrentPage);
	
	if UNLIKELY_IF(!Destination)
	{
		return;
	}
	
	PublishToPage(*Destination, FullState);
	EntityPages.Add(Update.NetworkId, Destination);
	MaterializedEntities.Add(Update.NetworkId, MoveTemp(FullState));
	
	if (CurrentPage && CurrentPage != Destination)
	{
		CurrentPage->RemoveEntity(Update.NetworkId);
		ScheduleRetirementIfEmpty(*CurrentPage);
	}
}

void UFlecsIrisReplicationTransport::MigrateEntity(const FFlecsReplicationRouteDescriptor&,
	const FFlecsReplicationRouteDescriptor& NewRoute, const FFlecsReplicationLayoutDefinition& Layout,
	const FFlecsReplicatedEntityUpdate& FullUpdate)
{
	UFlecsIrisReplicationShard* OldPage = EntityPages.FindRef(FullUpdate.NetworkId);
	UFlecsIrisReplicationShard* Destination = FindPageForUpdate(NewRoute, FullUpdate, OldPage);
	if (!Destination)
	{
		return;
	}
	Destination->UpsertLayout(Layout);
	Destination->UpsertEntity(FullUpdate);
	EntityPages.Add(FullUpdate.NetworkId, Destination);
	MaterializedEntities.Add(FullUpdate.NetworkId, FullUpdate);
	if (OldPage && OldPage != Destination)
	{
		OldPage->RemoveEntity(FullUpdate.NetworkId);
		ScheduleRetirementIfEmpty(*OldPage);
	}
}

void UFlecsIrisReplicationTransport::RemoveEntity(const FFlecsReplicationRouteDescriptor&,
	const FFlecsNetworkId NetworkId)
{
	if (UFlecsIrisReplicationShard* Page = EntityPages.FindRef(NetworkId))
	{
		Page->RemoveEntity(NetworkId);
		EntityPages.Remove(NetworkId);
		MaterializedEntities.Remove(NetworkId);
		ScheduleRetirementIfEmpty(*Page);
	}
}

void UFlecsIrisReplicationTransport::HandleProtocolError(const FString& Diagnostic)
{
	Super::HandleProtocolError(Diagnostic);
	
	if (const UFlecsNetworkWorldSubsystem* Subsystem = GetNetworkSubsystem())
	{
		const UNetDriver* Driver = Subsystem->GetWorld()->GetNetDriver(); 
		
		if (Driver && Driver->ServerConnection)
		{
			Driver->ServerConnection->Close();
		}
	}
}

UFlecsIrisReplicationShard* UFlecsIrisReplicationTransport::FindPageForUpdate(
	const FFlecsReplicationRouteDescriptor& Route, const FFlecsReplicatedEntityUpdate& Update,
	UFlecsIrisReplicationShard* ExcludedPage)
{
	for (UFlecsIrisReplicationShard* Page : Pages)
	{
		if (Page && Page != ExcludedPage && Page->GetRouteDescriptor() == Route && Page->CanFitUpdate(Update))
		{
			return Page;
		}
	}
	
	return CreatePage(Route);
}

UFlecsIrisReplicationShard* UFlecsIrisReplicationTransport::CreatePage(
	const FFlecsReplicationRouteDescriptor& Route)
{
	
	UFlecsNetworkWorldSubsystem* Subsystem = GetNetworkSubsystem();
	if (!Subsystem || !Subsystem->HasAuthority())
	{
		return nullptr;
	}
	
	const TSolidNotNull<UFlecsIrisReplicationShard*> Page = NewObject<UFlecsIrisReplicationShard>(Subsystem);
	const TSolidNotNull<const UFlecsNetworkingModuleSettings*> Settings = GetDefault<UFlecsNetworkingModuleSettings>();
	
	Page->InitializeServer(Subsystem->GetWorld(), Route,
		Route.PollFrequency > 0.0f ? Route.PollFrequency : Settings->DefaultShardPollFrequency,
		Route.StaticPriority > 0.0f ? Route.StaticPriority : Settings->DefaultShardStaticPriority,
		Route.PageEntityLimit > 0 ? Route.PageEntityLimit : Settings->DefaultPageEntityLimit,
		Route.PageRetainedPayloadByteLimit > 0 ? Route.PageRetainedPayloadByteLimit
			: Settings->DefaultPageRetainedPayloadByteLimit);
	Pages.Add(Page);
	
	return Page;
}

void UFlecsIrisReplicationTransport::PublishToPage(UFlecsIrisReplicationShard& Page,
	const FFlecsReplicatedEntityUpdate& Update)
{
	if (const FFlecsReplicationLayoutDefinition* Layout = Layouts.Find(Update.LayoutId))
	{
		Page.UpsertLayout(*Layout);
	}
	Page.UpsertEntity(Update);
	PendingRetirementTicks.Remove(&Page);
}

void UFlecsIrisReplicationTransport::ScheduleRetirementIfEmpty(UFlecsIrisReplicationShard& Page)
{
	if (Page.IsEmpty())
	{
		PendingRetirementTicks.FindOrAdd(&Page) = 0;
	}
}

FFlecsReplicatedEntityUpdate UFlecsIrisReplicationTransport::MaterializeUpdate(
	const FFlecsReplicatedEntityUpdate& Update) const
{
	const FFlecsReplicatedEntityUpdate* Previous = MaterializedEntities.Find(Update.NetworkId);
	if (Update.Kind == EFlecsReplicatedEntityUpdateKind::Full || !Previous
		|| Previous->LayoutId != Update.LayoutId)
	{
		FFlecsReplicatedEntityUpdate Result = Update;
		Result.Kind = EFlecsReplicatedEntityUpdateKind::Full;
		return Result;
	}

	FFlecsReplicatedEntityUpdate Result = *Previous;
	Result.StateRevision = Update.StateRevision;
	Result.CompositionRevision = Update.CompositionRevision;
	Result.Route = Update.Route;
	Result.ChangedKeys.Reset();
	
	for (const FFlecsReplicatedValue& Value : Update.Values)
	{
		if (FFlecsReplicatedValue* Existing = Result.Values.FindByPredicate(
			[&Value](const FFlecsReplicatedValue& Candidate)
			{
				return Candidate.KeyIndex == Value.KeyIndex;
			}))
		{
			*Existing = Value;
		}
		else
		{
			Result.Values.Add(Value);
		}
	}
	
	for (const FFlecsReplicatedValue& Value : Result.Values)
	{
		Result.ChangedKeys.Add(Value.KeyIndex);
	}
	
	Result.Kind = EFlecsReplicatedEntityUpdateKind::Full;
	return Result;
}
