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
#include "Networking/FlecsNetworkingStats.h"
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
	for (const TPair<FName, TObjectPtr<UFlecsIrisReplicationShard>>& Pair : Pages)
	{
		if (Pair.Value)
		{
			Pair.Value->StopReplication();
		}
	}
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
	Pages.Reset();
	RoutePageNames.Reset();
	EntityPageNames.Reset();
	MaterializedEntities.Reset();
	RouteLayouts.Reset();
	PendingRetirements.Reset();
	Super::ShutdownTransport();
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

void UFlecsIrisReplicationTransport::TickTransport()
{
	EnsureReplicationFilter();
	TArray<FName> ReadyToRetire;
	for (TPair<FName, uint8>& Pair : PendingRetirements)
	{
		if (Pair.Value == 0)
		{
			ReadyToRetire.Add(Pair.Key);
		}
		else
		{
			--Pair.Value;
		}
	}
	for (const FName PageName : ReadyToRetire)
	{
		RetirePage(PageName);
	}
	SET_DWORD_STAT(STAT_FlecsReplicationPages, Pages.Num());

	bool bAllStarted = true;
	for (const TPair<FName, TObjectPtr<UFlecsIrisReplicationShard>>& Pair : Pages)
	{
		if (Pair.Value)
		{
			bAllStarted &= Pair.Value->TryStartReplication();
		}
	}
	if (!Pages.IsEmpty() && !bAllStarted && ++StartAttempts > 120 && !bWarnedReplicationSystemUnavailable)
	{
		bWarnedReplicationSystemUnavailable = true;
		UE_LOG(LogFlecsCore, Warning,
			TEXT("UnrealFlecs Iris pages could not attach to a compatible ReplicationSystem. Enable Iris on the active NetDriver; Flecs simulation remains active without network transport."));
	}
}

void UFlecsIrisReplicationTransport::PublishLayout(const FFlecsReplicationRouteDescriptor& Route,
	const FFlecsReplicationLayoutDefinition& Layout)
{
	RouteLayouts.FindOrAdd(Route.LogicalKey.Name).Add(Layout.LayoutId, Layout);
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
	for (const FFlecsReplicatedValue& Value : Update.Values)
	{
		if (FFlecsReplicatedValue* Existing = Result.Values.FindByPredicate(
			[&Value](const FFlecsReplicatedValue& Candidate) { return Candidate.KeyIndex == Value.KeyIndex; }))
		{
			*Existing = Value;
		}
		else
		{
			Result.Values.Add(Value);
		}
	}
	Result.Kind = EFlecsReplicatedEntityUpdateKind::Full;
	return Result;
}

void UFlecsIrisReplicationTransport::PublishEntity(const FFlecsReplicationRouteDescriptor& Route,
	const FFlecsReplicatedEntityUpdate& Update)
{
	FFlecsReplicatedEntityUpdate Materialized = MaterializeUpdate(Update);
	
	const uint32 PayloadBytes = Materialized.GetPayloadByteCount();

	const FName CurrentPageName = EntityPageNames.FindRef(Update.NetworkId);
	
	UFlecsIrisReplicationShard* CurrentPage = CurrentPageName.IsNone() ? nullptr
		: Pages.FindRef(CurrentPageName).Get();
	
	UFlecsIrisReplicationShard* Destination = CurrentPage;
	
	const UFlecsNetworkingModuleSettings* Settings = GetDefault<UFlecsNetworkingModuleSettings>();
	
	const uint32 EntityLimit = Route.PageEntityLimit != 0 ? Route.PageEntityLimit : Settings->DefaultPageEntityLimit;
	const uint32 ByteLimit = Route.PageByteLimit != 0 ? Route.PageByteLimit : Settings->DefaultPageByteLimit;
	
	const uint32 CurrentEntityBytes = CurrentPage && CurrentPage->FindMaterializedEntity(Update.NetworkId)
		? CurrentPage->FindMaterializedEntity(Update.NetworkId)->GetPayloadByteCount() : 0;
	
	const bool bFitsCurrent = CurrentPage && CurrentPage->GetRouteDescriptor() == Route
		&& (uint32)CurrentPage->GetEntityCount() <= EntityLimit
		&& CurrentPage->GetMaterializedPayloadBytes() - CurrentEntityBytes + PayloadBytes <= ByteLimit;
	
	if (!bFitsCurrent)
	{
		Destination = FindPageWithCapacity(Route, Update.NetworkId, PayloadBytes, CurrentPageName);
	}
	
	if (!Destination)
	{
		return;
	}
	
	const TMap<FFlecsReplicationLayoutId, FFlecsReplicationLayoutDefinition>* Layouts =
		RouteLayouts.Find(Route.LogicalKey.Name);
	const FFlecsReplicationLayoutDefinition* Layout = Layouts ? Layouts->Find(Update.LayoutId) : nullptr;
	if (!Layout)
	{
		return;
	}
	
	Destination->UpsertLayout(*Layout);
	
	if (Destination != CurrentPage)
	{
		Destination->UpsertEntity(Materialized);
		const FName DestinationName = FindPageName(Destination);
		EntityPageNames.Add(Update.NetworkId, DestinationName);
		MaterializedEntities.Add(Update.NetworkId, Materialized);
		PendingRetirements.Remove(DestinationName);
		if (CurrentPage)
		{
			CurrentPage->RemoveEntity(Update.NetworkId);
			QueueRetirementIfEmpty(CurrentPageName);
			INC_DWORD_STAT(STAT_FlecsReplicationMigrations);
		}
		return;
	}
	
	Destination->UpsertEntity(Update);
	MaterializedEntities.Add(Update.NetworkId, MoveTemp(Materialized));
}

void UFlecsIrisReplicationTransport::MigrateEntity(const FFlecsReplicationRouteDescriptor& OldRoute,
	const FFlecsReplicationRouteDescriptor& NewRoute, const FFlecsReplicationLayoutDefinition& Layout,
	const FFlecsReplicatedEntityUpdate& FullUpdate)
{
	PublishLayout(NewRoute, Layout);
	const FName OldPageName = EntityPageNames.FindRef(FullUpdate.NetworkId);
	UFlecsIrisReplicationShard* OldPage = OldPageName.IsNone() ? nullptr : Pages.FindRef(OldPageName).Get();
	UFlecsIrisReplicationShard* NewPage = FindPageWithCapacity(NewRoute, FullUpdate.NetworkId,
		FullUpdate.GetPayloadByteCount(), OldPageName);
	if (!NewPage)
	{
		return;
	}
	NewPage->UpsertLayout(Layout);
	NewPage->UpsertEntity(FullUpdate);
	const FName NewPageName = FindPageName(NewPage);
	EntityPageNames.Add(FullUpdate.NetworkId, NewPageName);
	MaterializedEntities.Add(FullUpdate.NetworkId, FullUpdate);
	PendingRetirements.Remove(NewPageName);
	if (OldPage)
	{
		OldPage->RemoveEntity(FullUpdate.NetworkId);
		QueueRetirementIfEmpty(OldPageName);
	}
}

void UFlecsIrisReplicationTransport::RemoveEntity(const FFlecsReplicationRouteDescriptor&,
	const FFlecsNetworkId NetworkId)
{
	const FName PageName = EntityPageNames.FindRef(NetworkId);
	if (UFlecsIrisReplicationShard* Page = Pages.FindRef(PageName).Get())
	{
		Page->RemoveEntity(NetworkId);
		QueueRetirementIfEmpty(PageName);
	}
	EntityPageNames.Remove(NetworkId);
	MaterializedEntities.Remove(NetworkId);
}

void UFlecsIrisReplicationTransport::SetEntityDormancy(const FFlecsReplicationRouteDescriptor&,
	const FFlecsNetworkId NetworkId, const bool bDormant)
{
	if (UFlecsIrisReplicationShard* Page = FindEntityPage(NetworkId))
	{
		Page->SetEntityDormant(NetworkId, bDormant);
	}
}

UFlecsIrisReplicationShard* UFlecsIrisReplicationTransport::FindPageWithCapacity(
	const FFlecsReplicationRouteDescriptor& Route, const FFlecsNetworkId NetworkId,
	const uint32 PayloadBytes, const FName ExcludedPage)
{
	const UFlecsNetworkingModuleSettings* Settings = GetDefault<UFlecsNetworkingModuleSettings>();
	const uint16 EntityLimit = Route.PageEntityLimit != 0 ? Route.PageEntityLimit : Settings->DefaultPageEntityLimit;
	const uint32 ByteLimit = Route.PageByteLimit != 0 ? Route.PageByteLimit : Settings->DefaultPageByteLimit;
	if (const TArray<FName>* Names = RoutePageNames.Find(Route.LogicalKey.Name))
	{
		for (const FName PageName : *Names)
		{
			if (PageName == ExcludedPage)
			{
				continue;
			}
			UFlecsIrisReplicationShard* Page = Pages.FindRef(PageName).Get();
			if (!Page || Page->GetRouteDescriptor() != Route)
			{
				continue;
			}
			const bool bAlreadyContained = Page->ContainsEntity(NetworkId);
			if ((!bAlreadyContained && Page->GetEntityCount() >= EntityLimit)
				|| Page->GetMaterializedPayloadBytes() + PayloadBytes > ByteLimit)
			{
				continue;
			}
			PendingRetirements.Remove(PageName);
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
	const uint32 PageIndex = NextPageIndices.FindOrAdd(Route.LogicalKey.Name)++;
	const FName PageName(*FString::Printf(TEXT("%s_%u"), *Route.LogicalKey.Name.ToString(), PageIndex));
	UFlecsIrisReplicationShard* Page = NewObject<UFlecsIrisReplicationShard>(Subsystem);
	Page->InitializeServer(Subsystem->GetWorld(), Route, PageIndex);
	Pages.Add(PageName, Page);
	RoutePageNames.FindOrAdd(Route.LogicalKey.Name).Add(PageName);
	return Page;
}

void UFlecsIrisReplicationTransport::QueueRetirementIfEmpty(const FName PageName)
{
	UFlecsIrisReplicationShard* Page = Pages.FindRef(PageName).Get();
	if (Page && Page->GetEntityCount() == 0)
	{
		PendingRetirements.Add(PageName, 1);
	}
}

void UFlecsIrisReplicationTransport::RetirePage(const FName PageName)
{
	UFlecsIrisReplicationShard* Page = Pages.FindRef(PageName).Get();
	if (!Page || Page->GetEntityCount() != 0)
	{
		PendingRetirements.Remove(PageName);
		return;
	}
	const FName RouteName = Page->GetRouteDescriptor().LogicalKey.Name;
	Page->StopReplication();
	Pages.Remove(PageName);
	if (TArray<FName>* Names = RoutePageNames.Find(RouteName))
	{
		Names->Remove(PageName);
		if (Names->IsEmpty())
		{
			RoutePageNames.Remove(RouteName);
		}
	}
	PendingRetirements.Remove(PageName);
}

FName UFlecsIrisReplicationTransport::FindPageName(const UFlecsIrisReplicationShard* Page) const
{
	for (const TPair<FName, TObjectPtr<UFlecsIrisReplicationShard>>& Pair : Pages)
	{
		if (Pair.Value == Page)
		{
			return Pair.Key;
		}
	}
	return NAME_None;
}

UFlecsIrisReplicationShard* UFlecsIrisReplicationTransport::FindEntityPage(
	const FFlecsNetworkId NetworkId) const
{
	return Pages.FindRef(EntityPageNames.FindRef(NetworkId)).Get();
}

int32 UFlecsIrisReplicationTransport::GetPageCount(const FName LogicalRoute) const
{
	return LogicalRoute.IsNone() ? Pages.Num() : RoutePageNames.FindRef(LogicalRoute).Num();
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
