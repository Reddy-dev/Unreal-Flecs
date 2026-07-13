// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsIrisReplicationShard.h"

#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectAdapter.h"
#include "Net/UnrealNetwork.h"

#include "Networking/FlecsIrisShardObjectFactory.h"
#include "Networking/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsIrisReplicationShard)

void FFlecsIrisLayoutManifestItem::PostReplicatedAdd(const FFlecsIrisLayoutManifest& Serializer)
{
	if (UFlecsIrisReplicationShard* Owner = Serializer.GetOwner()) Owner->EnqueueReceivedLayout(Definition);
}

void FFlecsIrisLayoutManifestItem::PostReplicatedChange(const FFlecsIrisLayoutManifest& Serializer)
{
	PostReplicatedAdd(Serializer);
}

void FFlecsIrisEntitySnapshotItem::PostReplicatedAdd(const FFlecsIrisEntitySnapshots& Serializer)
{
	if (UFlecsIrisReplicationShard* Owner = Serializer.GetOwner()) Owner->EnqueueReceivedEntity(Snapshot);
}

void FFlecsIrisEntitySnapshotItem::PostReplicatedChange(const FFlecsIrisEntitySnapshots& Serializer)
{
	PostReplicatedAdd(Serializer);
}

void FFlecsIrisEntitySnapshotItem::PreReplicatedRemove(const FFlecsIrisEntitySnapshots& Serializer)
{
	if (UFlecsIrisReplicationShard* Owner = Serializer.GetOwner()) Owner->EnqueueRemovedEntity(Snapshot.NetworkId);
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
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsIrisReplicationShard, ShardKey, InitialParams);

	FDoRepLifetimeParams PushParams;
	PushParams.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsIrisReplicationShard, LayoutManifest, PushParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsIrisReplicationShard, EntitySnapshots, PushParams);
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
	if (RootObjectAdapter) RootObjectAdapter->FillRootObjectReplicationParams(Context, OutParams);
	OutParams.PollFrequency = PollFrequency;
	OutParams.StaticPriority = StaticPriority;
	OutParams.bUseClassConfigDynamicFilter = false;
}

void UFlecsIrisReplicationShard::InitializeServer(UWorld* InWorld,
	const FFlecsReplicationRouteKey& InShardKey, const float InPollFrequency, const float InStaticPriority)
{
	BoundWorld = InWorld;
	ShardKey = InShardKey;
	PollFrequency = InPollFrequency;
	StaticPriority = InStaticPriority;
	LayoutManifest.SetOwner(this);
	EntitySnapshots.SetOwner(this);

	UE::Net::FRootObjectSettings Settings;
	Settings.bIsAlwaysRelevant = true;
	Settings.FactoryName = UFlecsIrisShardObjectFactory::GetFactoryName();
	RootObjectAdapter = MakeUnique<UE::Net::FNetRootObjectAdapter>(Settings);
	RootObjectAdapter->InitAdapter(this);
	RootObjectAdapter->SetNetFactoryName(UFlecsIrisShardObjectFactory::GetFactoryName());
}

void UFlecsIrisReplicationShard::BindClient(UWorld* InWorld)
{
	BoundWorld = InWorld;
	NetworkSubsystem = InWorld ? InWorld->GetSubsystem<UFlecsNetworkWorldSubsystem>() : nullptr;
	LayoutManifest.SetOwner(this);
	EntitySnapshots.SetOwner(this);
}

bool UFlecsIrisReplicationShard::TryStartReplication()
{
	if (!RootObjectAdapter || RootObjectAdapter->IsReplicating()) return RootObjectAdapter != nullptr;
	UWorld* World = GetWorld();
	UNetDriver* NetDriver = World ? World->GetNetDriver() : nullptr;
	if (!NetDriver || !NetDriver->GetReplicationSystem() || !World->PersistentLevel) return false;
	RootObjectAdapter->StartReplication(World->PersistentLevel);
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
	EntitySnapshots.SetOwner(nullptr);
}

void UFlecsIrisReplicationShard::UpsertLayout(const FFlecsReplicationLayoutDefinition& Layout)
{
	if (LayoutIndices.Contains(Layout.LayoutId)) return;
	FFlecsIrisLayoutManifestItem& Item = LayoutManifest.Items.AddDefaulted_GetRef();
	Item.Definition = Layout;
	LayoutIndices.Add(Layout.LayoutId, LayoutManifest.Items.Num() - 1);
	LayoutManifest.MarkItemDirty(Item);
}

void UFlecsIrisReplicationShard::UpsertEntity(const FFlecsReplicatedEntitySnapshot& Snapshot)
{
	if (int32* Index = EntityIndices.Find(Snapshot.NetworkId))
	{
		FFlecsIrisEntitySnapshotItem& Item = EntitySnapshots.Items[*Index];
		Item.Snapshot = Snapshot;
		EntitySnapshots.MarkItemDirty(Item);
		return;
	}
	FFlecsIrisEntitySnapshotItem& Item = EntitySnapshots.Items.AddDefaulted_GetRef();
	Item.Snapshot = Snapshot;
	EntityIndices.Add(Snapshot.NetworkId, EntitySnapshots.Items.Num() - 1);
	EntitySnapshots.MarkItemDirty(Item);
}

void UFlecsIrisReplicationShard::RemoveEntity(const FFlecsNetworkId NetworkId)
{
	const int32* Index = EntityIndices.Find(NetworkId);
	if (!Index) return;
	EntitySnapshots.Items.RemoveAt(*Index);
	EntityIndices.Remove(NetworkId);
	for (int32 ItemIndex = *Index; ItemIndex < EntitySnapshots.Items.Num(); ++ItemIndex)
	{
		EntityIndices.FindChecked(EntitySnapshots.Items[ItemIndex].Snapshot.NetworkId) = ItemIndex;
	}
	EntitySnapshots.MarkArrayDirty();
}

void UFlecsIrisReplicationShard::EnqueueAllReceived()
{
	for (const FFlecsIrisLayoutManifestItem& Item : LayoutManifest.Items) EnqueueReceivedLayout(Item.Definition);
	for (const FFlecsIrisEntitySnapshotItem& Item : EntitySnapshots.Items) EnqueueReceivedEntity(Item.Snapshot);
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

void UFlecsIrisReplicationShard::EnqueueReceivedEntity(const FFlecsReplicatedEntitySnapshot& Snapshot) const
{
	if (UFlecsNetworkWorldSubsystem* Subsystem = NetworkSubsystem.Get())
	{
		FFlecsReplicationInboxRecord Record;
		Record.Type = EFlecsReplicationInboxRecordType::UpsertEntity;
		Record.SourceShard = GetSourceShardId();
		Record.Snapshot = Snapshot;
		Subsystem->EnqueueReceivedRecord(MoveTemp(Record));
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

FGuid UFlecsIrisReplicationShard::GetSourceShardId() const
{
	return FFlecsReplicationSchemaId::FromStableName(TEXT("FlecsShard:") + ShardKey.Name.ToString()).Value;
}
