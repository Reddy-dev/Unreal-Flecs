// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsIrisReplicationShard.h"

#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Iris/ReplicationSystem/ObjectReplicationBridge.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Iris/ReplicationSystem/ReplicationSystem.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectAdapter.h"
#include "Net/UnrealNetwork.h"

#include "Networking/FlecsIrisReplicationFilter.h"
#include "Networking/FlecsIrisShardObjectFactory.h"
#include "Networking/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsIrisReplicationShard)

namespace
{
	constexpr int32 MaxRetainedUpdateItems = 512;
}

void FFlecsIrisLayoutManifestItem::PostReplicatedAdd(const FFlecsIrisLayoutManifest& Serializer)
{
	if (const UFlecsIrisReplicationShard* Owner = Serializer.GetOwner())
	{
		Owner->EnqueueReceivedLayout(Definition);
	}
}

void FFlecsIrisLayoutManifestItem::PostReplicatedChange(const FFlecsIrisLayoutManifest& Serializer)
{
	PostReplicatedAdd(Serializer);
}

void FFlecsIrisLayoutManifestItem::PreReplicatedRemove(const FFlecsIrisLayoutManifest& Serializer)
{
	if (const UFlecsIrisReplicationShard* Owner = Serializer.GetOwner())
	{
		Owner->EnqueueRemovedLayout(Definition);
	}
}

FFlecsNetworkId FFlecsIrisEntityBaselineItem::GetNetworkId() const
{
	return Chunks.IsEmpty() ? FFlecsNetworkId() : Chunks[0].NetworkId;
}

void FFlecsIrisEntityBaselineItem::PostReplicatedAdd(const FFlecsIrisEntityBaselines& Serializer)
{
	if (const UFlecsIrisReplicationShard* Owner = Serializer.GetOwner())
	{
		Owner->EnqueueReceivedChunks(Chunks);
	}
}

void FFlecsIrisEntityBaselineItem::PostReplicatedChange(const FFlecsIrisEntityBaselines& Serializer)
{
	PostReplicatedAdd(Serializer);
}

void FFlecsIrisEntityBaselineItem::PreReplicatedRemove(const FFlecsIrisEntityBaselines& Serializer)
{
	if (const UFlecsIrisReplicationShard* Owner = Serializer.GetOwner())
	{
		Owner->EnqueueRemovedEntity(GetNetworkId());
	}
}

void FFlecsIrisEntityUpdateItem::PostReplicatedAdd(const FFlecsIrisEntityUpdateStream& Serializer)
{
	if (const UFlecsIrisReplicationShard* Owner = Serializer.GetOwner())
	{
		Owner->EnqueueReceivedChunks(Chunks);
	}
}

void FFlecsIrisEntityUpdateItem::PostReplicatedChange(const FFlecsIrisEntityUpdateStream& Serializer)
{
	PostReplicatedAdd(Serializer);
}

UWorld* UFlecsIrisReplicationShard::GetWorld() const
{
	return BoundWorld.IsValid() ? BoundWorld.Get() : Super::GetWorld();
}

void UFlecsIrisReplicationShard::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams InitialParams;
	InitialParams.bIsPushBased = true;
	InitialParams.Condition = COND_InitialOnly;
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsIrisReplicationShard, PageDescriptor, InitialParams);

	FDoRepLifetimeParams PushParams;
	PushParams.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsIrisReplicationShard, LayoutManifest, PushParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsIrisReplicationShard, EntityBaselines, PushParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsIrisReplicationShard, EntityUpdateStream, PushParams);
}

void UFlecsIrisReplicationShard::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context,
	UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}

void UFlecsIrisReplicationShard::FillRootObjectReplicationParams(
	const UE::Net::FRootObjectReplicationParamsContext& Context,
	UE::Net::FRootObjectReplicationParams& OutParams) const
{
	if (RootObjectAdapter)
	{
		RootObjectAdapter->FillRootObjectReplicationParams(Context, OutParams);
	}
	OutParams.PollFrequency = PageDescriptor.Route.PollFrequency;
	OutParams.StaticPriority = PageDescriptor.Route.StaticPriority;
	OutParams.bIsDormant = bRootDormant;
	OutParams.bUseClassConfigDynamicFilter = false;
	OutParams.bUseExplicitDynamicFilter = true;
	OutParams.ExplicitDynamicFilterName = UFlecsIrisReplicationFilter::GetFilterName();
}

void UFlecsIrisReplicationShard::InitializeServer(UWorld* InWorld,
	const FFlecsReplicationRouteDescriptor& InRoute, const uint32 InPageIndex)
{
	BoundWorld = InWorld;
	PageDescriptor.Route = InRoute;
	PageDescriptor.PageIndex = InPageIndex;
	PageDescriptor.SourceShardId = FGuid::NewGuid();
	LayoutManifest.SetOwner(this);
	EntityBaselines.SetOwner(this);
	EntityUpdateStream.SetOwner(this);

	UE::Net::FRootObjectSettings Settings;
	Settings.bIsNotRouted = true;
	Settings.FactoryName = UFlecsIrisShardObjectFactory::GetFactoryName();
	RootObjectAdapter = MakeUnique<UE::Net::FNetRootObjectAdapter>(Settings);
	RootObjectAdapter->InitAdapter(this);
	RootObjectAdapter->SetNetFactoryName(UFlecsIrisShardObjectFactory::GetFactoryName());
}

void UFlecsIrisReplicationShard::BindClient(const TSolidNotNull<UWorld*> InWorld)
{
	BoundWorld = InWorld;
	NetworkSubsystem = InWorld->GetSubsystem<UFlecsNetworkWorldSubsystem>();
	if (UFlecsNetworkWorldSubsystem* Subsystem = NetworkSubsystem.Get())
	{
		FString Error;
		if (!Subsystem->ValidateInterestBinding(PageDescriptor.Route.Interest, Error))
		{
			if (UFlecsReplicationTransportBase* Transport = Subsystem->GetReplicationTransport())
			{
				Transport->HandleProtocolError(FString::Printf(
					TEXT("Received invalid Flecs replication page interest binding: %s"), *Error));
			}
		}
	}
	LayoutManifest.SetOwner(this);
	EntityBaselines.SetOwner(this);
	EntityUpdateStream.SetOwner(this);
}

bool UFlecsIrisReplicationShard::TryStartReplication()
{
	if UNLIKELY_IF(!RootObjectAdapter)
	{
		return false;
	}
	const UWorld* World = GetWorld();
	UNetDriver* NetDriver = World ? World->GetNetDriver() : nullptr;
	UReplicationSystem* ReplicationSystem = NetDriver ? NetDriver->GetReplicationSystem() : nullptr;
	if UNLIKELY_IF(!ReplicationSystem || !World->PersistentLevel)
	{
		return false;
	}
	if (!RootObjectAdapter->IsReplicating())
	{
		RootObjectAdapter->StartReplication(World->PersistentLevel);
	}
	if (RootObjectAdapter->IsReplicating())
	{
		if (UFlecsIrisReplicationFilter* Filter = Cast<UFlecsIrisReplicationFilter>(
			ReplicationSystem->GetFilter(UFlecsIrisReplicationFilter::GetFilterName())))
		{
			const UE::Net::FNetRefHandle Handle = ReplicationSystem->GetReplicationBridge()
				->GetReplicatedRefHandle(this);
			Filter->RegisterPage(Handle, this);
		}
	}
	return RootObjectAdapter->IsReplicating();
}

void UFlecsIrisReplicationShard::StopReplication()
{
	if (RootObjectAdapter)
	{
		RootObjectAdapter->StopReplication();
		RootObjectAdapter->DeinitAdapter();
		RootObjectAdapter.Reset();
	}
}

void UFlecsIrisReplicationShard::DetachedFromReplication()
{
	if (UFlecsNetworkWorldSubsystem* Subsystem = NetworkSubsystem.Get())
	{
		FFlecsReplicationInboxRecord Record;
		Record.Type = EFlecsReplicationInboxRecordType::DetachShard;
		Record.SourceShard = GetSourceShardId();
		Subsystem->EnqueueReceivedRecord(MoveTemp(Record));
	}
	NetworkSubsystem.Reset();
	LayoutManifest.SetOwner(nullptr);
	EntityBaselines.SetOwner(nullptr);
	EntityUpdateStream.SetOwner(nullptr);
}

void UFlecsIrisReplicationShard::UpsertLayout(const FFlecsReplicationLayoutDefinition& Layout)
{
	if (LayoutIndices.Contains(Layout.LayoutId))
	{
		return;
	}
	FFlecsIrisLayoutManifestItem& Item = LayoutManifest.Items.AddDefaulted_GetRef();
	Item.Definition = Layout;
	LayoutIndices.Add(Layout.LayoutId, LayoutManifest.Items.Num() - 1);
	LayoutManifest.MarkItemDirty(Item);
}

const FFlecsReplicatedEntityUpdate* UFlecsIrisReplicationShard::FindMaterializedEntity(
	const FFlecsNetworkId NetworkId) const
{
	return MaterializedEntities.Find(NetworkId);
}

void UFlecsIrisReplicationShard::UpsertEntity(const FFlecsReplicatedEntityUpdate& Update)
{
	FFlecsReplicatedEntityUpdate Materialized;
	const FFlecsReplicatedEntityUpdate* Previous = MaterializedEntities.Find(Update.NetworkId);
	const FFlecsReplicationLayoutId PreviousLayoutId = Previous ? Previous->LayoutId : FFlecsReplicationLayoutId();
	const uint32 PreviousBytes = Previous ? Previous->GetPayloadByteCount() : 0;
	if (Update.Kind == EFlecsReplicatedEntityUpdateKind::Full || !Previous || Previous->LayoutId != Update.LayoutId)
	{
		Materialized = Update;
	}
	else
	{
		Materialized = *Previous;
		Materialized.StateRevision = Update.StateRevision;
		Materialized.CompositionRevision = Update.CompositionRevision;
		Materialized.Route = Update.Route;
		for (const FFlecsReplicatedValue& Value : Update.Values)
		{
			if (FFlecsReplicatedValue* Existing = Materialized.Values.FindByPredicate(
				[&Value](const FFlecsReplicatedValue& Candidate) { return Candidate.KeyIndex == Value.KeyIndex; }))
			{
				*Existing = Value;
			}
			else
			{
				Materialized.Values.Add(Value);
			}
		}
	}
	Materialized.Kind = EFlecsReplicatedEntityUpdateKind::Full;
	MaterializedPayloadBytes = MaterializedPayloadBytes >= PreviousBytes
		? MaterializedPayloadBytes - PreviousBytes
		: 0;
	MaterializedPayloadBytes += Materialized.GetPayloadByteCount();
	MaterializedEntities.Add(Update.NetworkId, Materialized);

	if (PreviousLayoutId != Update.LayoutId)
	{
		if (PreviousLayoutId.IsValid())
		{
			--LayoutReferenceCounts.FindOrAdd(PreviousLayoutId);
		}
		++LayoutReferenceCounts.FindOrAdd(Update.LayoutId);
	}

	TArray<FFlecsReplicationUpdateChunk> BaselineChunks;
	BuildFlecsReplicationUpdateChunks(Materialized, BaselineChunks);
	if (const int32* Index = EntityIndices.Find(Update.NetworkId))
	{
		EntityBaselines.Items[*Index].Chunks = MoveTemp(BaselineChunks);
		EntityBaselines.MarkItemDirty(EntityBaselines.Items[*Index]);
	}
	else
	{
		FFlecsIrisEntityBaselineItem& Item = EntityBaselines.Items.AddDefaulted_GetRef();
		Item.Chunks = MoveTemp(BaselineChunks);
		EntityIndices.Add(Update.NetworkId, EntityBaselines.Items.Num() - 1);
		EntityBaselines.MarkItemDirty(Item);
	}

	FFlecsIrisEntityUpdateItem& StreamItem = EntityUpdateStream.Items.AddDefaulted_GetRef();
	BuildFlecsReplicationUpdateChunks(Update, StreamItem.Chunks);
	EntityUpdateStream.MarkItemDirty(StreamItem);
	if (EntityUpdateStream.Items.Num() > MaxRetainedUpdateItems)
	{
		EntityUpdateStream.Items.RemoveAt(0, EntityUpdateStream.Items.Num() - MaxRetainedUpdateItems);
		EntityUpdateStream.MarkArrayDirty();
	}

	if (PreviousLayoutId.IsValid() && PreviousLayoutId != Update.LayoutId)
	{
		RemoveLayoutIfUnused(PreviousLayoutId);
	}
	DormantEntities.Remove(Update.NetworkId);
	RefreshRootDormancy();
}

void UFlecsIrisReplicationShard::RemoveEntity(const FFlecsNetworkId NetworkId)
{
	const int32* Index = EntityIndices.Find(NetworkId);
	if (!Index)
	{
		return;
	}
	const FFlecsReplicatedEntityUpdate* Materialized = MaterializedEntities.Find(NetworkId);
	const FFlecsReplicationLayoutId LayoutId = Materialized ? Materialized->LayoutId : FFlecsReplicationLayoutId();
	if (Materialized)
	{
		const uint32 RemovedBytes = Materialized->GetPayloadByteCount();
		MaterializedPayloadBytes = MaterializedPayloadBytes >= RemovedBytes
			? MaterializedPayloadBytes - RemovedBytes
			: 0;
	}
	EntityBaselines.Items.RemoveAt(*Index);
	EntityIndices.Remove(NetworkId);
	MaterializedEntities.Remove(NetworkId);
	DormantEntities.Remove(NetworkId);
	for (int32 ItemIndex = *Index; ItemIndex < EntityBaselines.Items.Num(); ++ItemIndex)
	{
		EntityIndices.FindChecked(EntityBaselines.Items[ItemIndex].GetNetworkId()) = ItemIndex;
	}
	EntityBaselines.MarkArrayDirty();
	if (LayoutId.IsValid())
	{
		--LayoutReferenceCounts.FindOrAdd(LayoutId);
		RemoveLayoutIfUnused(LayoutId);
	}
	RefreshRootDormancy();
}

void UFlecsIrisReplicationShard::SetEntityDormant(const FFlecsNetworkId NetworkId, const bool bDormant)
{
	if (!EntityIndices.Contains(NetworkId))
	{
		return;
	}
	if (bDormant)
	{
		DormantEntities.Add(NetworkId);
	}
	else
	{
		DormantEntities.Remove(NetworkId);
	}
	
	RefreshRootDormancy();
}

void UFlecsIrisReplicationShard::RemoveLayoutIfUnused(const FFlecsReplicationLayoutId LayoutId)
{
	if (LayoutReferenceCounts.FindRef(LayoutId) > 0)
	{
		return;
	}
	const int32* Index = LayoutIndices.Find(LayoutId);
	if (!Index)
	{
		return;
	}
	LayoutManifest.Items.RemoveAt(*Index);
	LayoutIndices.Remove(LayoutId);
	LayoutReferenceCounts.Remove(LayoutId);
	for (int32 ItemIndex = *Index; ItemIndex < LayoutManifest.Items.Num(); ++ItemIndex)
	{
		LayoutIndices.FindChecked(LayoutManifest.Items[ItemIndex].Definition.LayoutId) = ItemIndex;
	}
	LayoutManifest.MarkArrayDirty();
}

void UFlecsIrisReplicationShard::RefreshRootDormancy()
{
	const bool bShouldBeDormant = !EntityIndices.IsEmpty() && DormantEntities.Num() == EntityIndices.Num();
	if (bShouldBeDormant == bRootDormant)
	{
		return;
	}
	bRootDormant = bShouldBeDormant;
	const UWorld* World = GetWorld();
	UNetDriver* NetDriver = World ? World->GetNetDriver() : nullptr;
	UReplicationSystem* ReplicationSystem = NetDriver ? NetDriver->GetReplicationSystem() : nullptr;
	UObjectReplicationBridge* Bridge = ReplicationSystem ? ReplicationSystem->GetReplicationBridge() : nullptr;
	
	if (!Bridge)
	{
		return;
	}
	
	const UE::Net::FNetRefHandle Handle = Bridge->GetReplicatedRefHandle(this);
	if (!Handle.IsValid())
	{
		return;
	}
	
	if (!bRootDormant)
	{
		Bridge->NetFlushDormantObject(Handle);
	}
	
	Bridge->SetObjectWantsToBeDormant(Handle, bRootDormant);
}

void UFlecsIrisReplicationShard::EnqueueAllReceived()
{
	for (const FFlecsIrisLayoutManifestItem& Item : LayoutManifest.Items)
	{
		EnqueueReceivedLayout(Item.Definition);
	}
	for (const FFlecsIrisEntityBaselineItem& Item : EntityBaselines.Items)
	{
		EnqueueReceivedChunks(Item.Chunks);
	}
}

void UFlecsIrisReplicationShard::EnqueueReceivedLayout(const FFlecsReplicationLayoutDefinition& Layout) const
{
	if (UFlecsNetworkWorldSubsystem* Subsystem = NetworkSubsystem.Get())
	{
		FFlecsReplicationInboxRecord Record;
		Record.Type = EFlecsReplicationInboxRecordType::Layout;
		Record.SourceShard = GetSourceShardId();
		Record.Layout = Layout;
		Subsystem->EnqueueReceivedRecord(MoveTemp(Record));
	}
}

void UFlecsIrisReplicationShard::EnqueueRemovedLayout(const FFlecsReplicationLayoutDefinition& Layout) const
{
	if (UFlecsNetworkWorldSubsystem* Subsystem = NetworkSubsystem.Get())
	{
		FFlecsReplicationInboxRecord Record;
		Record.Type = EFlecsReplicationInboxRecordType::RemoveLayout;
		Record.SourceShard = GetSourceShardId();
		Record.Layout = Layout;
		Subsystem->EnqueueReceivedRecord(MoveTemp(Record));
	}
}

void UFlecsIrisReplicationShard::EnqueueReceivedChunks(
	const TArray<FFlecsReplicationUpdateChunk>& Chunks) const
{
	if (UFlecsNetworkWorldSubsystem* Subsystem = NetworkSubsystem.Get())
	{
		for (const FFlecsReplicationUpdateChunk& Chunk : Chunks)
		{
			FFlecsReplicationInboxRecord Record;
			Record.Type = EFlecsReplicationInboxRecordType::UpdateChunk;
			Record.SourceShard = GetSourceShardId();
			Record.Chunk = Chunk;
			Subsystem->EnqueueReceivedRecord(MoveTemp(Record));
		}
	}
}

void UFlecsIrisReplicationShard::EnqueueRemovedEntity(const FFlecsNetworkId NetworkId) const
{
	if (UFlecsNetworkWorldSubsystem* Subsystem = NetworkSubsystem.Get())
	{
		FFlecsReplicationInboxRecord Record;
		Record.Type = EFlecsReplicationInboxRecordType::RemoveEntity;
		Record.SourceShard = GetSourceShardId();
		Record.NetworkId = NetworkId;
		Subsystem->EnqueueReceivedRecord(MoveTemp(Record));
	}
}
