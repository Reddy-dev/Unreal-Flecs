// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsNetworkWorldSubsystem.h"

#include "Engine/World.h"

#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

#include "Networking/FlecsNetRoleType.h"
#include "Networking/FlecsNetworkingModuleSettings.h"
#include "Networking/FlecsNetworkSubsystemSingleton.h"
#include "Networking/FlecsReplicatedEntityComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetworkWorldSubsystem)

UFlecsNetworkWorldSubsystem::UFlecsNetworkWorldSubsystem()
	: NetworkIdAllocator(1)
	, Router(MakeUnique<FFlecsDefaultReplicationRouter>())
{
}

void UFlecsNetworkWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	PreActorTickHandle = FWorldDelegates::OnWorldPreActorTick.AddUObject(
		this, &UFlecsNetworkWorldSubsystem::HandleWorldPreActorTick);
}

void UFlecsNetworkWorldSubsystem::OnFlecsWorldInitialized(const TSolidNotNull<UFlecsWorld*> InWorld)
{
	Super::OnFlecsWorldInitialized(InWorld);
	
	InWorld->Set<FFlecsNetworkSubsystemSingleton>(FFlecsNetworkSubsystemSingleton{ this });

	const uint8 Epoch = static_cast<uint8>((FPlatformTime::Cycles64() ^ PointerHash(this)) & 0xFFu);
	NetworkIdAllocator.Reset(Epoch == 0 ? 1 : Epoch);
	
	InstallDirtyObservers();
	CreateReplicationTransport();
}

void UFlecsNetworkWorldSubsystem::Deinitialize()
{
	if (PreActorTickHandle.IsValid())
	{
		FWorldDelegates::OnWorldPreActorTick.Remove(PreActorTickHandle);
		PreActorTickHandle.Reset();
	}
	
	if (ReplicationTransport)
	{
		ReplicationTransport->ShutdownTransport();
		ReplicationTransport = nullptr;
	}
	
	// Observer handles are non-owning. The Flecs world owns and destroys the
	// observer entities with the world. At UWorld subsystem teardown the Flecs
	// world may already be finalizing, so querying or destructing these handles
	// would call back into an inaccessible ecs_world_t.
	DirtyObservers.Reset();
	
	if (GetFlecsWorld() && DescriptorRegisteredHandle.IsValid())
	{
		FFlecsComponentReplicationRegistry::Get(GetFlecsWorld()).OnDescriptorRegistered().Remove(DescriptorRegisteredHandle);
		DescriptorRegisteredHandle.Reset();
	}
	
	FFlecsComponentReplicationRegistry::RemoveWorld(GetFlecsWorld());
	
	Super::Deinitialize();
}

FFlecsNetworkId UFlecsNetworkWorldSubsystem::BeginReplicatingEntity(const FFlecsEntityHandle& EntityHandle)
{
	if UNLIKELY_IF(!HasAuthority())
	{
#if WITH_AUTOMATION_TESTS || WITH_EDITOR
		if (bForceClientModeForTesting)
		{
			return {};
		}
#endif
		
		UE_LOG(LogFlecsCore, Error, TEXT("BeginReplicatingEntity called without authority"));
		return {};
	}
	
	if UNLIKELY_IF(!EntityHandle.IsValid())
	{
		UE_LOG(LogFlecsCore, Error, TEXT("BeginReplicatingEntity called with an invalid entity"));
		return {};
	}
	
	const FFlecsNetworkId* Existing = EntityHandle.TryGet<FFlecsNetworkId>(); 

	if (Existing && Existing->IsValid())
	{
		NetworkIdToEntityHandleMap.FindOrAdd(*Existing) = EntityHandle;
		EntityStates.FindOrAdd(*Existing);
		DirtyEntities.Add(*Existing);
		return *Existing;
	}

	const FFlecsNetworkId NetworkId = NetworkIdAllocator.Allocate();
	
	if UNLIKELY_IF(!NetworkId.IsValid())
	{
		UE_LOG(LogFlecsCore, Error, TEXT("Flecs network ID allocator exhausted its slot space"));
		return {};
	}
	
	EntityHandle.Set<FFlecsNetworkId>(NetworkId);
	EntityHandle.Add<EFlecsNetRoleType>(EFlecsNetRoleType::Authority);
	
	NetworkIdToEntityHandleMap.Add(NetworkId, EntityHandle);
	EntityStates.Add(NetworkId);
	DirtyEntities.Add(NetworkId);
	
	return NetworkId;
}

void UFlecsNetworkWorldSubsystem::StopReplicatingEntity(const FFlecsEntityHandle& EntityHandle)
{
	if UNLIKELY_IF(!HasAuthority())
	{
		return;
	}
	
	if LIKELY_IF(EntityHandle.IsValid())
	{
		if (const FFlecsNetworkId* NetworkId = EntityHandle.TryGet<FFlecsNetworkId>();
			NetworkId && NetworkId->IsValid())
		{
			StopReplicatingEntity(*NetworkId);
			return;
		}
	}

	// During an entity-delete OnRemove event Flecs can already report the handle
	// as invalid. The stable local ID is still present in the reverse index.
	const FFlecsId LocalEntityId = EntityHandle.GetFlecsId();
	FFlecsNetworkId FoundNetworkId;
	
	for (const TPair<FFlecsNetworkId, FFlecsEntityHandle>& Pair : NetworkIdToEntityHandleMap)
	{
		if (Pair.Value.GetFlecsId() == LocalEntityId)
		{
			FoundNetworkId = Pair.Key;
			break;
		}
	}
	
	if (FoundNetworkId.IsValid())
	{
		StopReplicatingEntity(FoundNetworkId);
	}
}

void UFlecsNetworkWorldSubsystem::StopReplicatingEntity(const FFlecsNetworkId NetworkId)
{
	if UNLIKELY_IF(!NetworkId.IsValid() || !HasAuthority())
	{
		return;
	}
	
	if (ReplicationTransport)
	{
		const FReplicatedEntityState* State = EntityStates.Find(NetworkId);
		ReplicationTransport->RemoveEntity(
			State ? State->RouteKey : FFlecsReplicationRouteKey::Default(), NetworkId);
	}
	
	const FFlecsEntityHandle* Entity = NetworkIdToEntityHandleMap.Find(NetworkId);
	
	if (IsValid(Entity) && Entity->Has<FFlecsNetworkId>())
	{
		Entity->Remove<FFlecsNetworkId>();
		Entity->Remove<EFlecsNetRoleType>();
	}
	
	DirtyEntities.Remove(NetworkId);
	EntityStates.Remove(NetworkId);
	NetworkIdToEntityHandleMap.Remove(NetworkId);
	NetworkIdAllocator.Release(NetworkId);
}

void UFlecsNetworkWorldSubsystem::MarkEntityDirty(const FFlecsEntityHandle& EntityHandle)
{
	if UNLIKELY_IF(!HasAuthority() || !EntityHandle.IsValid())
	{
		return;
	}
	
	const FFlecsNetworkId* NetworkId = EntityHandle.TryGet<FFlecsNetworkId>(); 
	
	if (NetworkId && NetworkId->IsValid())
	{
		DirtyEntities.Add(*NetworkId);
	}
}

FFlecsEntityHandle UFlecsNetworkWorldSubsystem::FindEntity(const FFlecsNetworkId NetworkId) const
{
	if (const FFlecsEntityHandle* Found = NetworkIdToEntityHandleMap.Find(NetworkId))
	{
		return *Found;
	}
	
	return FFlecsEntityHandle::Invalid();
}

bool UFlecsNetworkWorldSubsystem::HasAuthority() const
{
#if WITH_AUTOMATION_TESTS || WITH_EDITOR
	if (bForceClientModeForTesting)
	{
		return false;
	}
#endif
	
	return GetWorld() && GetWorld()->GetNetMode() != NM_Client;
}

void UFlecsNetworkWorldSubsystem::CreateReplicationTransport()
{
	if (GetWorld()->GetNetMode() == NM_Standalone)
	{
		return;
	}
	
	const FName ProviderName = GetNetworkingModuleSettings()->ReplicationProviderName;
	const UClass* ProviderClass = FFlecsReplicationTransportRegistry::FindProvider(ProviderName);
	
	if UNLIKELY_IF(!ProviderClass)
	{
		UE_LOG(LogFlecsCore, Error,
			TEXT("Flecs replication provider '%s' is unavailable. Enable Iris and the UnrealFlecsIris runtime module; standalone Flecs remains active."),
			*ProviderName.ToString());
		return;
	}
	
	ReplicationTransport = NewObject<UFlecsReplicationTransportBase>(this, ProviderClass);
	
	if UNLIKELY_IF(!ReplicationTransport || !ReplicationTransport->InitializeTransport(this))
	{
		UE_LOG(LogFlecsCore, Error,
			TEXT("Flecs replication provider '%s' could not initialize for this NetDriver. Replication is inactive."),
			*ProviderName.ToString());
		
		ReplicationTransport = nullptr;
	}
}

void UFlecsNetworkWorldSubsystem::InstallDirtyObservers()
{
	if (!HasAuthority())
	{
		return;
	}
	
	const TSolidNotNull<UFlecsWorld*> World = GetFlecsWorldChecked();
	FFlecsComponentReplicationRegistry& Registry = FFlecsComponentReplicationRegistry::Get(World);
	
	DescriptorRegisteredHandle = Registry.OnDescriptorRegistered().AddUObject(
		this, &UFlecsNetworkWorldSubsystem::InstallDirtyObserversForDescriptor);
	
	for (const TPair<FFlecsId, FFlecsComponentReplicationDescriptor>& Pair : Registry.GetDescriptors())
	{
		InstallDirtyObserversForDescriptor(Pair.Value);
	}
}

void UFlecsNetworkWorldSubsystem::InstallDirtyObserversForDescriptor(
	const FFlecsComponentReplicationDescriptor& Descriptor)
{
	if (!HasAuthority() || !IsFlecsWorldValid())
	{
		return;
	}

	const TSolidNotNull<UFlecsWorld*> World = GetFlecsWorldChecked();
	const FFlecsId MarkerId = World->GetIdIfRegistered<FFlecsReplicatedEntityComponent>();
	
	if UNLIKELY_IF(!World->IsValidId(Descriptor.LocalFlecsId))
	{
		UE_LOG(LogFlecsCore, Error,
			TEXT("Cannot install replication observers for schema '%s': Flecs ID %llu does not belong to this world"),
			*Descriptor.StableName, Descriptor.LocalFlecsId.GetId());
		return;
	}
	
	if UNLIKELY_IF(!MarkerId.IsValid() || !World->IsValidId(MarkerId))
	{
		UE_LOG(LogFlecsCore, Error,
			TEXT("Cannot install replication observers for schema '%s': replicated-entity marker is not registered in this world"),
			*Descriptor.StableName);
		return;
	}
	
	const TWeakObjectPtr<UFlecsNetworkWorldSubsystem> WeakThis(this);
	
	auto Install = [World, WeakThis, MarkerId](const flecs::id_t ObservedId)
	{
		const flecs::observer Observer = World->GetNativeFlecsWorld().observer()
			.with(ObservedId)
			.with(MarkerId.GetId())
			.event(flecs::OnAdd)
			.event(flecs::OnSet)
			.event(flecs::OnRemove)
			.each([WeakThis](flecs::entity Entity)
		{
				// @TODO: maybe utilize the singleton?
			if (UFlecsNetworkWorldSubsystem* Subsystem = WeakThis.Get())
			{
				Subsystem->MarkEntityDirty(FFlecsEntityHandle(Entity));
			}
		});
		
		return Observer;
	};
	
	DirtyObservers.Add(Install(Descriptor.LocalFlecsId.GetId()));
	DirtyObservers.Add(Install(FFlecsId::MakePair(Descriptor.LocalFlecsId.GetId(), flecs::Wildcard)));
	
	// @TODO: is this needed?
	//DirtyObservers.Add(Install(FFlecsId::MakePair(flecs::Wildcard, Descriptor.LocalFlecsId.GetId())));
}

void UFlecsNetworkWorldSubsystem::GatherDirtyEntities()
{
	if (DirtyEntities.IsEmpty())
	{
		return;
	}
	
	TArray<FFlecsNetworkId> Pending = DirtyEntities.Array();
	DirtyEntities.Reset();
	
	const TSolidNotNull<UFlecsWorld*> World = GetFlecsWorldChecked();
	
	for (const FFlecsNetworkId NetworkId : Pending)
	{
		const FFlecsEntityHandle* EntityPtr = NetworkIdToEntityHandleMap.Find(NetworkId);
		
		if UNLIKELY_IF(!EntityPtr || !EntityPtr->IsValid())
		{
			continue;
		}
		
		const FFlecsEntityHandle Entity = *EntityPtr;
		
		bool bLayoutCreated = false;
		FString Error;
		const FFlecsReplicationLayoutDefinition* Layout = LayoutRegistry.BuildForEntity(
			World, Entity, bLayoutCreated, Error);
		
		if UNLIKELY_IF(!Layout)
		{
			UE_LOG(LogFlecsCore, Error, TEXT("Failed to gather entity %llu: %s"), NetworkId.GetValue(), *Error);
			continue;
		}

		FReplicatedEntityState& State = EntityStates.FindOrAdd(NetworkId);
		
		const FFlecsReplicationRouteKey Route = Router->Route(Entity);
		
		if (State.LayoutId != Layout->LayoutId)
		{
			State.LayoutId = Layout->LayoutId;
			++State.CompositionRevision;
		}
		
		State.RouteKey = Route;
		++State.StateRevision;

		const FString PublishedKey = Route.Name.ToString() + TEXT("|") + Layout->LayoutId.ToString();
		if (!PublishedLayoutRoutes.Contains(PublishedKey) && ReplicationTransport)
		{
			ReplicationTransport->PublishLayout(Route, *Layout);
			PublishedLayoutRoutes.Add(PublishedKey);
		}

		FFlecsReplicatedEntitySnapshot Snapshot;
		Snapshot.NetworkId = NetworkId;
		Snapshot.StateRevision = State.StateRevision;
		Snapshot.CompositionRevision = State.CompositionRevision;
		Snapshot.LayoutId = Layout->LayoutId;
		Snapshot.RouteKey = Route;

		for (int32 KeyIndex = 0; KeyIndex < Layout->Keys.Num(); ++KeyIndex)
		{
			const FFlecsReplicationKey& Key = Layout->Keys[KeyIndex];
			
			if (Key.StorageKind == EFlecsReplicationKeyStorageKind::None)
			{
				continue;
			}
			
			FFlecsId LocalId;
			
			if (!ResolveKeyToLocalId(Key, LocalId))
			{
				continue;
			}
			
			const FFlecsComponentReplicationDescriptor* Descriptor = Key.TryGetStorageDescriptor(World);
			const void* Value = Entity.TryGet(LocalId);
			
			if UNLIKELY_IF(!Descriptor || !Value)
			{
				continue;
			}
			
			FFlecsReplicatedValue& Serialized = Snapshot.Values.AddDefaulted_GetRef();
			
			Serialized.KeyIndex = static_cast<uint16>(KeyIndex);
			FMemoryWriter Writer(Serialized.Bytes, true);
			
			if UNLIKELY_IF(!Descriptor->Serialize(Writer, const_cast<void*>(Value)) || Writer.IsError())
			{
				UE_LOG(LogFlecsCore, Error, TEXT("Failed to serialize schema '%s' for entity %llu"),
					*Descriptor->StableName, NetworkId.GetValue());
				Snapshot.Values.Pop();
			}
		}
		
		if (ReplicationTransport)
		{
			ReplicationTransport->PublishEntity(Route, Snapshot);
		}
	}
}

void UFlecsNetworkWorldSubsystem::DrainInbox()
{
	FFlecsReplicationInboxRecord Record;
	
	while (Inbox.Dequeue(Record))
	{
		switch (Record.Type)
		{
		case EFlecsReplicationInboxRecordType::Layout:
		{
			FString Error;
			if (!ValidateLayout(Record.Layout, Error) || !LayoutRegistry.AddRemoteDefinition(Record.Layout, Error))
			{
				if (ReplicationTransport)
				{
					ReplicationTransport->HandleProtocolError(Error);
				}
				break;
			}
				
			if (TArray<TPair<FGuid, FFlecsReplicatedEntitySnapshot>>* Deferred = DeferredSnapshots.Find(Record.Layout.LayoutId))
			{
				TArray<TPair<FGuid, FFlecsReplicatedEntitySnapshot>> Pending = MoveTemp(*Deferred);
				DeferredSnapshots.Remove(Record.Layout.LayoutId);
				
				for (const TPair<FGuid, FFlecsReplicatedEntitySnapshot>& Item : Pending)
				{
					ApplySnapshot(Item.Key, Item.Value);
				}
			}
				
			break;
		}
		case EFlecsReplicationInboxRecordType::UpsertEntity:
			ApplySnapshot(Record.SourceShard, Record.Snapshot);
			break;
		case EFlecsReplicationInboxRecordType::RemoveEntity:
			RemoveRemoteEntity(Record.NetworkId);
			break;
		case EFlecsReplicationInboxRecordType::DetachShard:
			DetachRemoteShard(Record.SourceShard);
			break;
		}
	}
	
	RetryEntityPairFixups();
}

void UFlecsNetworkWorldSubsystem::ApplySnapshot(const FGuid& SourceShard,
	const FFlecsReplicatedEntitySnapshot& Snapshot)
{
	const FFlecsReplicationLayoutDefinition* Layout = LayoutRegistry.Find(Snapshot.LayoutId);
	
	if (!Layout)
	{
		DeferredSnapshots.FindOrAdd(Snapshot.LayoutId).Emplace(SourceShard, Snapshot);
		return;
	}
	
	if (const uint32* Revision = LastAppliedStateRevisions.Find(Snapshot.NetworkId);
		Revision && *Revision >= Snapshot.StateRevision)
	{
		return;
	}
	
	const FFlecsNetworkId* Bound = ClientSlotBindings.Find(Snapshot.NetworkId.GetSlot()); 

	if (Bound && *Bound != Snapshot.NetworkId)
	{
		if (Bound->GetValue() > Snapshot.NetworkId.GetValue())
		{
			return;
		}
		
		RemoveRemoteEntity(*Bound);
	}

	// Accepted snapshots are complete replacements. Any unresolved pairs from
	// an older state revision no longer describe this source's current state.
	EntityPairFixups.RemoveAll([&Snapshot](const FEntityPairFixup& Fixup)
	{
		return Fixup.Source == Snapshot.NetworkId;
	});

	const TSolidNotNull<UFlecsWorld*> World = GetFlecsWorldChecked();
	FFlecsEntityHandle Entity = FindEntity(Snapshot.NetworkId);
	
	if (!Entity.IsValid())
	{
		Entity = World->CreateEntity();
		Entity.Set<FFlecsNetworkId>(Snapshot.NetworkId);
		Entity.Add<FFlecsReplicatedEntityComponent>();
		Entity.Add<EFlecsNetRoleType>(EFlecsNetRoleType::SimulatedProxy);
		
		NetworkIdToEntityHandleMap.Add(Snapshot.NetworkId, Entity);
		ClientSlotBindings.Add(Snapshot.NetworkId.GetSlot(), Snapshot.NetworkId);
	}

	TSet<FFlecsId> DesiredIds;
	for (int32 KeyIndex = 0; KeyIndex < Layout->Keys.Num(); ++KeyIndex)
	{
		const FFlecsReplicationKey& Key = Layout->Keys[KeyIndex];
		FFlecsId LocalId;
		
		// @TODO
		if (ResolveKeyToLocalId(Key, LocalId))
		{
			DesiredIds.Add(LocalId);
		}
		else if (Key.Secondary.Kind == EFlecsReplicationPairTargetKind::Entity)
		{
			TOptional<TArray<uint8>> Payload;
			const FFlecsReplicatedValue* Value = Snapshot.Values.FindByPredicate(
				[KeyIndex](const FFlecsReplicatedValue& Candidate)
				{
					return Candidate.KeyIndex == KeyIndex;
				});

			if (Value)
			{
				Payload = Value->Bytes;
			}

			EntityPairFixups.Add({ Snapshot.NetworkId, Key.Secondary.Entity, Key,
				Snapshot.StateRevision, MoveTemp(Payload) });
		}
	}

	const FFlecsComponentReplicationRegistry& Registry = FFlecsComponentReplicationRegistry::Get(World);
	World->Defer([&]()
	{
		TArray<FFlecsId> ToRemove;
		
		for (const FFlecsId CurrentId : Entity.GetType())
		{
			const bool bReplicated = CurrentId.IsPair()
				? Registry.Find(CurrentId.GetFirst()) != nullptr
				: Registry.Find(CurrentId) != nullptr;
			
			if (bReplicated && !DesiredIds.Contains(CurrentId))
			{
				ToRemove.Add(CurrentId);
			}
		}
		
		for (const FFlecsId Id : ToRemove)
		{
			Entity.Remove(Id);
		}
		
		for (const FFlecsId Id : DesiredIds)
		{
			Entity.Add(Id);
		}

		for (const FFlecsReplicatedValue& Value : Snapshot.Values)
		{
			if (!Layout->Keys.IsValidIndex(Value.KeyIndex))
			{
				continue;
			}
			
			const FFlecsReplicationKey& Key = Layout->Keys[Value.KeyIndex];
			FFlecsId LocalId;
			
			if (!ResolveKeyToLocalId(Key, LocalId))
			{
				continue;
			}
			
			ApplyResolvedValue(Entity, LocalId, Key, &Value.Bytes);
		}
	});

	LastAppliedStateRevisions.Add(Snapshot.NetworkId, Snapshot.StateRevision);
	EntitySourceShards.Add(Snapshot.NetworkId, SourceShard);
}

void UFlecsNetworkWorldSubsystem::RemoveRemoteEntity(const FFlecsNetworkId NetworkId)
{
	const FFlecsEntityHandle* Entity = NetworkIdToEntityHandleMap.Find(NetworkId); 
	
	if (IsValid(Entity))
	{
		Entity->Destroy();
	}
	
	NetworkIdToEntityHandleMap.Remove(NetworkId);
	ClientSlotBindings.Remove(NetworkId.GetSlot());
	LastAppliedStateRevisions.Remove(NetworkId);
	EntitySourceShards.Remove(NetworkId);
	
	EntityPairFixups.RemoveAll([NetworkId](const FEntityPairFixup& Fixup)
	{
		return Fixup.Source == NetworkId || Fixup.Target == NetworkId;
	});
}

void UFlecsNetworkWorldSubsystem::DetachRemoteShard(const FGuid& SourceShard)
{
	TArray<FFlecsNetworkId> ToRemove;
	
	for (const TPair<FFlecsNetworkId, FGuid>& Pair : EntitySourceShards)
	{
		if (Pair.Value == SourceShard)
		{
			ToRemove.Add(Pair.Key);
		}
	}
	
	for (const FFlecsNetworkId Id : ToRemove)
	{
		RemoveRemoteEntity(Id);
	}
}

void UFlecsNetworkWorldSubsystem::RetryEntityPairFixups()
{
	const TSolidNotNull<UFlecsWorld*> World = GetFlecsWorldChecked();
	World->Defer([&]()
	{
		for (int32 Index = EntityPairFixups.Num() - 1; Index >= 0; --Index)
		{
			const FEntityPairFixup& Fixup = EntityPairFixups[Index];
			const FFlecsEntityHandle Source = FindEntity(Fixup.Source);
			const uint32* AcceptedRevision = LastAppliedStateRevisions.Find(Fixup.Source);

			if (!Source.IsValid() || !AcceptedRevision || *AcceptedRevision != Fixup.StateRevision)
			{
				EntityPairFixups.RemoveAtSwap(Index, 1, EAllowShrinking::No);
				continue;
			}

			FFlecsId PairId;

			if (!ResolveKeyToLocalId(Fixup.Key, PairId))
			{
				continue;
			}

			ApplyResolvedValue(Source, PairId, Fixup.Key,
				Fixup.Payload.IsSet() ? &Fixup.Payload.GetValue() : nullptr);
			EntityPairFixups.RemoveAtSwap(Index, 1, EAllowShrinking::No);
		}
	});
}

void UFlecsNetworkWorldSubsystem::ApplyResolvedValue(const FFlecsEntityHandle& Entity,
	const FFlecsId LocalId, const FFlecsReplicationKey& Key, const TArray<uint8>* Payload) const
{
	Entity.Add(LocalId);

	if (!Payload)
	{
		return;
	}

	const FFlecsComponentReplicationRegistry& Registry = FFlecsComponentReplicationRegistry::Get(GetFlecsWorldChecked());
	
	const EFlecsReplicationKeyStorageKind StorageKind = Key.StorageKind;
	
	if UNLIKELY_IF(!ensureAlwaysMsgf(StorageKind != EFlecsReplicationKeyStorageKind::None,
		TEXT("Cannot apply resolved value for schema {'%s'}: storage kind is None"), *Key.CanonicalString()))
	{
		return;
	}
	
	const FFlecsComponentReplicationDescriptor* Descriptor = Key.TryGetStorageDescriptor(GetFlecsWorldChecked());

	// @TODO: maybe a check?
	// how tf
	if UNLIKELY_IF(!Descriptor || Descriptor->bIsTag)
	{
		UE_LOGFMT(LogFlecsCore, Error, 
			"Cannot apply resolved value for schema {String}: storage descriptor is missing or is a tag",
			*Key.CanonicalString());
		return;
	}

	void* Temp = FMemory::Malloc(Descriptor->Size, Descriptor->Alignment);
	Descriptor->Construct(Temp);

	FMemoryReader Reader(*Payload, true);
	const bool bRead = Descriptor->Deserialize(Reader, Temp) && !Reader.IsError();

	if (bRead)
	{
		Entity.Set(LocalId, Descriptor->Size, Temp);
	}

	Descriptor->Destroy(Temp);
	FMemory::Free(Temp);
}

bool UFlecsNetworkWorldSubsystem::ResolveIndividualKeyToLocalId(const FFlecsReplicationIndividualKey& Key,
	FFlecsId& OutId) const
{
	const TSolidNotNull<const UFlecsWorld*> World = GetFlecsWorldChecked();
	const FFlecsComponentReplicationRegistry& Registry = FFlecsComponentReplicationRegistry::Get(World);
	
	switch (Key.Kind)
	{
		case EFlecsReplicationPairTargetKind::None:
			//Error
			break;
		case EFlecsReplicationPairTargetKind::Schema:
			{
				if LIKELY_IF(const FFlecsComponentReplicationDescriptor* Descriptor = Registry.Find(Key.Schema))
				{
					OutId = Descriptor->LocalFlecsId;
					return true;
				}
				break;
			}
		case EFlecsReplicationPairTargetKind::StableSymbolValue:
			{
				const FFlecsEntityHandle StableTarget = World->LookupEntityBySymbol_Internal(Key.StableIdentifier);
				if LIKELY_IF(StableTarget.IsValid())
				{
					OutId = StableTarget.GetFlecsId();
					return true;
				}
				
				break;
			}
		case EFlecsReplicationPairTargetKind::StablePathValue:
			{
				const FFlecsEntityHandle StableTarget = World->LookupEntity(Key.StableIdentifier);
				if LIKELY_IF(StableTarget.IsValid())
				{
					OutId = StableTarget.GetFlecsId();
					return true;
				}
				
				break;
			}
			break;
		case EFlecsReplicationPairTargetKind::Entity:
			{
				const FFlecsEntityHandle EntityTarget = FindEntity(Key.Entity);
				if LIKELY_IF(EntityTarget.IsValid())
				{
					OutId = EntityTarget.GetFlecsId();
					return true;
				}
				
				break;
			}
	}
	
	OutId = FFlecsId::Null();
	return false;
}

bool UFlecsNetworkWorldSubsystem::ResolveKeyToLocalId(const FFlecsReplicationKey& Key, FFlecsId& OutId) const
{
	const TSolidNotNull<const UFlecsWorld*> World = GetFlecsWorldChecked();
	const FFlecsComponentReplicationRegistry& Registry = FFlecsComponentReplicationRegistry::Get(World);
	
	if (Key.Kind == EFlecsReplicationKeyKind::Component)
	{
		return ResolveIndividualKeyToLocalId(Key.Primary, OutId);
	}
	
	FFlecsId Relationship;
	FFlecsId Target;
	
	const bool bResolvedRelationship = ResolveIndividualKeyToLocalId(Key.Primary, Relationship);
	
	if UNLIKELY_IF(!bResolvedRelationship)
	{
		return false;
	}
	
	const bool bResolvedTarget = ResolveIndividualKeyToLocalId(Key.Secondary, Target);
	if UNLIKELY_IF(!bResolvedTarget)
	{
		return false;
	}
	
	OutId = FFlecsId::MakePair(Relationship.GetId(), Target.GetId());
	return true;
}

bool UFlecsNetworkWorldSubsystem::ValidateReplicationIndividualKey(const FFlecsReplicationIndividualKey& Key,
	FString& OutError) const
{
	switch (Key.Kind)
	{
		case EFlecsReplicationPairTargetKind::None:
			OutError = TEXT("Replication individual key has no target kind");
			return false;
		case EFlecsReplicationPairTargetKind::Schema:
			if (!Key.Schema.IsValid())
			{
				OutError = TEXT("Replication individual key has invalid schema");
				return false;
			}
			break;
		case EFlecsReplicationPairTargetKind::StableSymbolValue:
		case EFlecsReplicationPairTargetKind::StablePathValue:
			if (Key.StableIdentifier.IsEmpty())
			{
				OutError = TEXT("Replication individual key has empty stable identifier");
				return false;
			}
			break;
		case EFlecsReplicationPairTargetKind::Entity:
			if (!Key.Entity.IsValid())
			{
				OutError = TEXT("Replication individual key has invalid entity target");
				return false;
			}
			break;
	}
	
	return true;
}

bool UFlecsNetworkWorldSubsystem::ValidateReplicationKey(const FFlecsReplicationKey& Key, FString& OutError) const
{
	if (Key.Kind == EFlecsReplicationKeyKind::Component)
	{
		return ValidateReplicationIndividualKey(Key.Primary, OutError);
	}
	else if (Key.Kind == EFlecsReplicationKeyKind::Pair)
	{
		if (!ValidateReplicationIndividualKey(Key.Primary, OutError))
		{
			return false;
		}
		
		if (!ValidateReplicationIndividualKey(Key.Secondary, OutError))
		{
			return false;
		}
	}
	
	return true;
}

bool UFlecsNetworkWorldSubsystem::ValidateLayout(const FFlecsReplicationLayoutDefinition& Layout, FString& OutError) const
{
	for (const FFlecsReplicationKey& Key : Layout.Keys)
	{
		if (!ValidateReplicationKey(Key, OutError))
		{
			return false;
		}
	}
	
	return true;
}

void UFlecsNetworkWorldSubsystem::HandleWorldPreActorTick(UWorld* World, ELevelTick, float)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_FlecsNetworkWorldSubsystem_HandleWorldPreActorTick);
	
	if UNLIKELY_IF(World != GetWorld() || !IsFlecsWorldValid())
	{
		return;
	}
	
	if (HasAuthority())
	{
		GatherDirtyEntities();
	}
	else
	{
		DrainInbox();
	}
	
	if (ReplicationTransport)
	{
		ReplicationTransport->TickTransport();
	}
}

TSolidNotNull<const UFlecsNetworkingModuleSettings*> UFlecsNetworkWorldSubsystem::GetNetworkingModuleSettings() const
{
	return GetDefault<UFlecsNetworkingModuleSettings>();
}
