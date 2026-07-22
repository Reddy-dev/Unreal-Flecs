// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsIrisReplicationShard.h"

#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Iris/ReplicationSystem/Filtering/NetObjectFilter.h"
#include "Iris/ReplicationSystem/ObjectReplicationBridge.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Iris/ReplicationSystem/ReplicationSystem.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectAdapter.h"
#include "Net/UnrealNetwork.h"
#include "Templates/Greater.h"

#include "Networking/FlecsIrisReplicationFilter.h"
#include "Networking/FlecsIrisShardObjectFactory.h"
#include "Networking/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsIrisReplicationShard)

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

void FFlecsIrisEntityHeaderItem::PostReplicatedAdd(const FFlecsIrisEntityHeaders& Serializer)
{
	if (UFlecsIrisReplicationShard* Owner = Serializer.GetOwner())
	{
		Owner->TryEnqueueReceivedEntity(Header.NetworkId);
	}
}

void FFlecsIrisEntityHeaderItem::PostReplicatedChange(const FFlecsIrisEntityHeaders& Serializer)
{
	PostReplicatedAdd(Serializer);
}

void FFlecsIrisEntityHeaderItem::PreReplicatedRemove(const FFlecsIrisEntityHeaders& Serializer)
{
	if (const UFlecsIrisReplicationShard* Owner = Serializer.GetOwner())
	{
		Owner->EnqueueRemovedEntity(Header.NetworkId);
	}
}

void FFlecsIrisEntityValueItem::PostReplicatedAdd(const FFlecsIrisEntityValues& Serializer)
{
	if (UFlecsIrisReplicationShard* Owner = Serializer.GetOwner())
	{
		Owner->TryEnqueueReceivedEntity(NetworkId);
	}
}

void FFlecsIrisEntityValueItem::PostReplicatedChange(const FFlecsIrisEntityValues& Serializer)
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
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsIrisReplicationShard, RouteDescriptor, InitialParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsIrisReplicationShard, SourceShardId, InitialParams);

	FDoRepLifetimeParams PushParams;
	PushParams.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsIrisReplicationShard, LayoutManifest, PushParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsIrisReplicationShard, EntityHeaders, PushParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsIrisReplicationShard, EntityValues, PushParams);
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
	
	OutParams.PollFrequency = PollFrequency;
	OutParams.StaticPriority = StaticPriority;
	OutParams.bUseClassConfigDynamicFilter = false;
}

void UFlecsIrisReplicationShard::InitializeServer(UWorld* InWorld,
	const FFlecsReplicationRouteDescriptor& InRoute, const float InPollFrequency,
	const float InStaticPriority, const uint16 InEntityLimit, const uint32 InByteLimit)
{
	BoundWorld = InWorld;
	RouteDescriptor = InRoute;
	SourceShardId = FGuid::NewGuid();
	PollFrequency = InPollFrequency;
	StaticPriority = InStaticPriority;
	EntityLimit = FMath::Max<uint16>(1, InEntityLimit);
	ByteLimit = FMath::Max<uint32>(1, InByteLimit);
	
	LayoutManifest.SetOwner(this);
	EntityHeaders.SetOwner(this);
	EntityValues.SetOwner(this);

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
	
	LayoutManifest.SetOwner(this);
	EntityHeaders.SetOwner(this);
	EntityValues.SetOwner(this);
}

bool UFlecsIrisReplicationShard::TryStartReplication()
{
	if UNLIKELY_IF(!RootObjectAdapter)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const UNetDriver* NetDriver = World ? World->GetNetDriver() : nullptr;
	
	if UNLIKELY_IF(!NetDriver || !NetDriver->GetReplicationSystem() || !World->PersistentLevel)
	{
		return false;
	}
	
	if (!RootObjectAdapter->IsReplicating())
	{
		RootObjectAdapter->StartReplication(World->PersistentLevel);
	}

	return RootObjectAdapter->IsReplicating() && TryAttachInterestFilter();
}

bool UFlecsIrisReplicationShard::TryAttachInterestFilter()
{
	if (bInterestFilterAttached)
	{
		return true;
	}

	UWorld* World = GetWorld();
	UNetDriver* NetDriver = World ? World->GetNetDriver() : nullptr;
	UReplicationSystem* ReplicationSystem = NetDriver ? NetDriver->GetReplicationSystem() : nullptr;
	UObjectReplicationBridge* ReplicationBridge = ReplicationSystem
		? ReplicationSystem->GetReplicationBridge() : nullptr;
	if (!ReplicationSystem || !ReplicationBridge)
	{
		return false;
	}

	const UE::Net::FNetRefHandle Handle = ReplicationBridge->GetReplicatedRefHandle(this);
	const UE::Net::FNetObjectFilterHandle FilterHandle = ReplicationSystem->GetFilterHandle(
		UFlecsIrisReplicationFilter::GetFilterName());
	UFlecsIrisReplicationFilter* Filter = Cast<UFlecsIrisReplicationFilter>(
		ReplicationSystem->GetFilter(UFlecsIrisReplicationFilter::GetFilterName()));
	if (!Handle.IsValid() || FilterHandle == UE::Net::InvalidNetObjectFilterHandle || !Filter)
	{
		return false;
	}

	if (!ReplicationSystem->SetFilter(Handle, FilterHandle))
	{
		return false;
	}

	Filter->RegisterPage(Handle, this);
	bInterestFilterAttached = true;
	return true;
}

void UFlecsIrisReplicationShard::StopReplication()
{
	bInterestFilterAttached = false;
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
	EntityHeaders.SetOwner(nullptr);
	EntityValues.SetOwner(nullptr);
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

void UFlecsIrisReplicationShard::UpsertEntity(const FFlecsReplicatedEntityUpdate& Update)
{
	FFlecsReplicatedEntityUpdate Header = Update;
	Header.Values.Reset();

	if (const int32* Index = EntityIndices.Find(Update.NetworkId))
	{
		FFlecsIrisEntityHeaderItem& Item = EntityHeaders.Items[*Index];
		Item.Header = MoveTemp(Header);
		EntityHeaders.MarkItemDirty(Item);
	}
	else
	{
		FFlecsIrisEntityHeaderItem& Item = EntityHeaders.Items.AddDefaulted_GetRef();
		Item.Header = MoveTemp(Header);
		EntityIndices.Add(Update.NetworkId, EntityHeaders.Items.Num() - 1);
		EntityHeaders.MarkItemDirty(Item);
	}

	TMap<uint16, int32>& ValueIndices = EntityValueIndices.FindOrAdd(Update.NetworkId);
	if (Update.Kind == EFlecsReplicatedEntityUpdateKind::Full)
	{
		TSet<uint16> RetainedKeys;
		for (const FFlecsReplicatedValue& Value : Update.Values) RetainedKeys.Add(Value.KeyIndex);
		TArray<uint16> RemovedKeys;
		for (const TPair<uint16, int32>& Pair : ValueIndices)
		{
			if (!RetainedKeys.Contains(Pair.Key)) RemovedKeys.Add(Pair.Key);
		}
		for (const uint16 Key : RemovedKeys)
		{
			const int32 RemovedIndex = ValueIndices.FindChecked(Key);
			RetainedPayloadBytes -= EntityValues.Items[RemovedIndex].Value.Bytes.Num();
			EntityValues.Items.RemoveAt(RemovedIndex);
			ValueIndices.Remove(Key);
			for (TPair<FFlecsNetworkId, TMap<uint16, int32>>& EntityPair : EntityValueIndices)
			{
				for (TPair<uint16, int32>& IndexPair : EntityPair.Value)
				{
					if (IndexPair.Value > RemovedIndex) --IndexPair.Value;
				}
			}
		}
	}

	for (const FFlecsReplicatedValue& Value : Update.Values)
	{
		if (int32* ExistingIndex = ValueIndices.Find(Value.KeyIndex))
		{
			FFlecsIrisEntityValueItem& Item = EntityValues.Items[*ExistingIndex];
			RetainedPayloadBytes -= Item.Value.Bytes.Num();
			Item.LayoutId = Update.LayoutId;
			Item.StateRevision = Update.StateRevision;
			Item.Value = Value;
			RetainedPayloadBytes += Item.Value.Bytes.Num();
			EntityValues.MarkItemDirty(Item);
		}
		else
		{
			FFlecsIrisEntityValueItem& Item = EntityValues.Items.AddDefaulted_GetRef();
			Item.NetworkId = Update.NetworkId;
			Item.LayoutId = Update.LayoutId;
			Item.StateRevision = Update.StateRevision;
			Item.Value = Value;
			ValueIndices.Add(Value.KeyIndex, EntityValues.Items.Num() - 1);
			RetainedPayloadBytes += Item.Value.Bytes.Num();
			EntityValues.MarkItemDirty(Item);
		}
	}
	uint32 EntityBytes = 0;
	for (const TPair<uint16, int32>& Pair : ValueIndices)
	{
		EntityBytes += EntityValues.Items[Pair.Value].Value.Bytes.Num();
	}
	EntityPayloadBytes.Add(Update.NetworkId, EntityBytes);
	if (EntityBytes > ByteLimit && !bWarnedOversizeEntity)
	{
		bWarnedOversizeEntity = true;
		UE_LOG(LogFlecsCore, Warning,
			TEXT("Flecs entity %llu retains %u payload bytes, exceeding its page limit of %u; keeping it on a dedicated page"),
			Update.NetworkId.GetValue(), EntityBytes, ByteLimit);
	}
}

void UFlecsIrisReplicationShard::RemoveEntity(const FFlecsNetworkId NetworkId)
{
	const int32* Index = EntityIndices.Find(NetworkId);
	
	if UNLIKELY_IF(!Index)
	{
		return;
	}
	
	EntityHeaders.Items.RemoveAt(*Index);
	EntityIndices.Remove(NetworkId);
	
	for (int32 ItemIndex = *Index; ItemIndex < EntityHeaders.Items.Num(); ++ItemIndex)
	{
		EntityIndices.FindChecked(EntityHeaders.Items[ItemIndex].Header.NetworkId) = ItemIndex;
	}
	
	EntityHeaders.MarkArrayDirty();

	TMap<uint16, int32> RemovedValueIndices;
	if (EntityValueIndices.RemoveAndCopyValue(NetworkId, RemovedValueIndices))
	{
		TArray<int32> Indices;
		RemovedValueIndices.GenerateValueArray(Indices);
		Indices.Sort(TGreater<int32>());
		for (const int32 RemovedIndex : Indices)
		{
			RetainedPayloadBytes -= EntityValues.Items[RemovedIndex].Value.Bytes.Num();
			EntityValues.Items.RemoveAt(RemovedIndex);
			for (TPair<FFlecsNetworkId, TMap<uint16, int32>>& EntityPair : EntityValueIndices)
			{
				for (TPair<uint16, int32>& IndexPair : EntityPair.Value)
				{
					if (IndexPair.Value > RemovedIndex) --IndexPair.Value;
				}
			}
		}
		EntityValues.MarkArrayDirty();
	}
	EntityPayloadBytes.Remove(NetworkId);
	ReceivedBaselines.Remove(NetworkId);
}

bool UFlecsIrisReplicationShard::CanFitUpdate(const FFlecsReplicatedEntityUpdate& Update) const
{
	const bool bExisting = EntityIndices.Contains(Update.NetworkId);
	if (!bExisting && EntityHeaders.Items.Num() >= EntityLimit)
	{
		return false;
	}

	uint32 NewEntityBytes = 0;
	if (Update.Kind == EFlecsReplicatedEntityUpdateKind::Full)
	{
		NewEntityBytes = Update.GetPayloadByteCount();
	}
	else
	{
		NewEntityBytes = EntityPayloadBytes.FindRef(Update.NetworkId);
		const TMap<uint16, int32>* ValueIndices = EntityValueIndices.Find(Update.NetworkId);
		
		for (const FFlecsReplicatedValue& Value : Update.Values)
		{
			const int32* Index = ValueIndices ? ValueIndices->Find(Value.KeyIndex) : nullptr;
			NewEntityBytes -= Index ? EntityValues.Items[*Index].Value.Bytes.Num() : 0;
			NewEntityBytes += Value.Bytes.Num();
		}
	}

	const uint32 ExistingBytes = EntityPayloadBytes.FindRef(Update.NetworkId);
	const uint64 NewPageBytes = static_cast<uint64>(RetainedPayloadBytes) - ExistingBytes + NewEntityBytes;
	const bool bDedicatedOversizePage = EntityHeaders.Items.IsEmpty() || (bExisting && EntityHeaders.Items.Num() == 1);
	return NewPageBytes <= ByteLimit || bDedicatedOversizePage;
}

void UFlecsIrisReplicationShard::EnqueueAllReceived()
{
	for (const FFlecsIrisLayoutManifestItem& Item : LayoutManifest.Items)
	{
		EnqueueReceivedLayout(Item.Definition);
	}
	
	for (const FFlecsIrisEntityHeaderItem& Item : EntityHeaders.Items)
	{
		TryEnqueueReceivedEntity(Item.Header.NetworkId);
	}
}

void UFlecsIrisReplicationShard::EnqueueReceivedLayout(const FFlecsReplicationLayoutDefinition& Layout) const
{
	if LIKELY_IF(UFlecsNetworkWorldSubsystem* Subsystem = NetworkSubsystem.Get())
	{
		FFlecsReplicationInboxRecord Record;
		Record.Type = EFlecsReplicationInboxRecordType::Layout;
		Record.SourceShard = GetSourceShardId();
		Record.Layout = Layout;
		Subsystem->EnqueueReceivedRecord(MoveTemp(Record));
	}
}

void UFlecsIrisReplicationShard::TryEnqueueReceivedEntity(const FFlecsNetworkId NetworkId)
{
	const FFlecsIrisEntityHeaderItem* HeaderItem = EntityHeaders.Items.FindByPredicate(
		[NetworkId](const FFlecsIrisEntityHeaderItem& Candidate)
		{
			return Candidate.Header.NetworkId == NetworkId;
		});
	if (!HeaderItem)
	{
		return;
	}

	FFlecsReplicatedEntityUpdate Update = HeaderItem->Header;
	const bool bFirstObservation = !ReceivedBaselines.Contains(NetworkId);
	
	if (bFirstObservation)
	{
		Update.Kind = EFlecsReplicatedEntityUpdateKind::Full;
		Update.ChangedKeys.Reset();
		for (const FFlecsIrisEntityValueItem& Item : EntityValues.Items)
		{
			if (Item.NetworkId == NetworkId && Item.LayoutId == Update.LayoutId)
			{
				Update.ChangedKeys.Add(Item.Value.KeyIndex);
				Update.Values.Add(Item.Value);
			}
		}

		const FFlecsIrisLayoutManifestItem* LayoutItem = LayoutManifest.Items.FindByPredicate(
			[&Update](const FFlecsIrisLayoutManifestItem& Candidate)
			{
				return Candidate.Definition.LayoutId == Update.LayoutId;
			});
		if (!LayoutItem)
		{
			return;
		}
		int32 ExpectedValueCount = 0;
		for (const FFlecsReplicationKey& Key : LayoutItem->Definition.Keys)
		{
			ExpectedValueCount += Key.StorageKind != EFlecsReplicationKeyStorageKind::None ? 1 : 0;
		}
		if (Update.Values.Num() != ExpectedValueCount)
		{
			return;
		}
	}
	else
	{
		for (const uint16 ChangedKey : Update.ChangedKeys)
		{
			const FFlecsIrisEntityValueItem* Item = EntityValues.Items.FindByPredicate(
				[&](const FFlecsIrisEntityValueItem& Candidate)
				{
					return Candidate.NetworkId == NetworkId && Candidate.LayoutId == Update.LayoutId
						&& Candidate.StateRevision == Update.StateRevision
						&& Candidate.Value.KeyIndex == ChangedKey;
				});
			if (!Item)
			{
				return;
			}
			Update.Values.Add(Item->Value);
		}
	}

	if LIKELY_IF(UFlecsNetworkWorldSubsystem* Subsystem = NetworkSubsystem.Get())
	{
		FFlecsReplicationInboxRecord Record;
		Record.Type = EFlecsReplicationInboxRecordType::UpsertEntity;
		Record.SourceShard = GetSourceShardId();
		Record.Update = MoveTemp(Update);
		Subsystem->EnqueueReceivedRecord(MoveTemp(Record));
		ReceivedBaselines.Add(NetworkId);
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
	return SourceShardId;
}
