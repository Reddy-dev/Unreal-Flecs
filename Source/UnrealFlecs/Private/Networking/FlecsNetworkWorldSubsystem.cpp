// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsNetworkWorldSubsystem.h"

#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

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
	if (!HasAuthority())
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
	if (!EntityHandle.IsValid())
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
	
	if (!NetworkId.IsValid())
	{
		UE_LOG(LogFlecsCore, Error, TEXT("Flecs network ID allocator exhausted its slot space"));
		return {};
	}
	
	EntityHandle.Set<FFlecsNetworkId>(NetworkId);
	NetworkIdToEntityHandleMap.Add(NetworkId, EntityHandle);
	EntityStates.Add(NetworkId);
	DirtyEntities.Add(NetworkId);
	return NetworkId;
}

void UFlecsNetworkWorldSubsystem::StopReplicatingEntity(const FFlecsEntityHandle& EntityHandle)
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (EntityHandle.IsValid())
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
	
	if (const FFlecsEntityHandle* Entity = NetworkIdToEntityHandleMap.Find(NetworkId);
		Entity && Entity->IsValid() && Entity->Has<FFlecsNetworkId>())
	{
		Entity->Remove<FFlecsNetworkId>();
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
	
	if (!World->IsValidId(Descriptor.LocalFlecsId))
	{
		UE_LOG(LogFlecsCore, Error,
			TEXT("Cannot install replication observers for schema '%s': Flecs ID %llu does not belong to this world"),
			*Descriptor.StableName, Descriptor.LocalFlecsId.GetId());
		return;
	}
	
	if (!MarkerId.IsValid() || !World->IsValidId(MarkerId))
	{
		UE_LOG(LogFlecsCore, Error,
			TEXT("Cannot install replication observers for schema '%s': replicated-entity marker is not registered in this world"),
			*Descriptor.StableName);
		return;
	}
	
	const TWeakObjectPtr<UFlecsNetworkWorldSubsystem> WeakThis(this);
	
	auto Install = [World, WeakThis, MarkerId](const flecs::id_t ObservedId)
	{
		flecs::observer Observer = World->GetNativeFlecsWorld().observer()
			.with(ObservedId)
			.with(MarkerId.GetId())
			.event(flecs::OnAdd)
			.event(flecs::OnSet)
			.event(flecs::OnRemove)
			.each([WeakThis](flecs::entity Entity)
		{
			if (UFlecsNetworkWorldSubsystem* Subsystem = WeakThis.Get())
			{
				Subsystem->MarkEntityDirty(FFlecsEntityHandle(Entity));
			}
		});
		
		return Observer;
	};
	
	DirtyObservers.Add(Install(Descriptor.LocalFlecsId.GetId()));
	DirtyObservers.Add(Install(ecs_pair(Descriptor.LocalFlecsId.GetId(), EcsWildcard)));
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
	
	FFlecsComponentReplicationRegistry& ComponentRegistry = FFlecsComponentReplicationRegistry::Get(World);

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
			
			if (!Key.bHasPayload)
			{
				continue;
			}
			
			FFlecsId LocalId;
			
			if (!ResolveKeyToLocalId(Key, LocalId))
			{
				continue;
			}
			
			const FFlecsComponentReplicationDescriptor* Descriptor = ComponentRegistry.Find(Key.StorageSchema);
			const void* Value = Entity.TryGet(LocalId);
			
			if (!Descriptor || !Value)
			{
				continue;
			}
			
			FFlecsReplicatedValue& Serialized = Snapshot.Values.AddDefaulted_GetRef();
			
			Serialized.KeyIndex = static_cast<uint16>(KeyIndex);
			FMemoryWriter Writer(Serialized.Bytes, true);
			
			if (!Descriptor->Serialize(Writer, const_cast<void*>(Value)) || Writer.IsError())
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
				
				for (const auto& Item : Pending)
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

	if (const FFlecsNetworkId* Bound = ClientSlotBindings.Find(Snapshot.NetworkId.GetSlot()); Bound && *Bound != Snapshot.NetworkId)
	{
		if (Bound->GetValue() > Snapshot.NetworkId.GetValue())
		{
			return;
		}
		
		RemoveRemoteEntity(*Bound);
	}

	UFlecsWorld* World = GetFlecsWorldChecked();
	FFlecsEntityHandle Entity = FindEntity(Snapshot.NetworkId);
	
	if (!Entity.IsValid())
	{
		Entity = World->CreateEntity();
		Entity.Set<FFlecsNetworkId>(Snapshot.NetworkId);
		Entity.Add<FFlecsReplicatedEntityComponent>();
		
		NetworkIdToEntityHandleMap.Add(Snapshot.NetworkId, Entity);
		ClientSlotBindings.Add(Snapshot.NetworkId.GetSlot(), Snapshot.NetworkId);
	}

	TSet<FFlecsId> DesiredIds;
	for (const FFlecsReplicationKey& Key : Layout->Keys)
	{
		FFlecsId LocalId;
		
		if (ResolveKeyToLocalId(Key, LocalId))
		{
			DesiredIds.Add(LocalId);
		}
		else if (Key.TargetKind == EFlecsReplicationPairTargetKind::Entity)
		{
			EntityPairFixups.Add({ Snapshot.NetworkId, Key.EntityTarget, Key });
		}
	}

	FFlecsComponentReplicationRegistry& Registry = FFlecsComponentReplicationRegistry::Get(World);
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
			
			const FFlecsComponentReplicationDescriptor* Descriptor = Registry.Find(Key.StorageSchema);
			if (!Descriptor || Descriptor->bIsTag)
			{
				continue;
			}
			
			void* Temp = FMemory::Malloc(Descriptor->Size, Descriptor->Alignment);
			Descriptor->Construct(Temp);
			
			FMemoryReader Reader(Value.Bytes, true);
			const bool bRead = Descriptor->Deserialize(Reader, Temp) && !Reader.IsError();
			
			if (bRead)
			{
				Entity.Set(LocalId, Descriptor->Size, Temp);
			}
			
			Descriptor->Destroy(Temp);
			FMemory::Free(Temp);
		}
	});

	LastAppliedStateRevisions.Add(Snapshot.NetworkId, Snapshot.StateRevision);
	EntitySourceShards.Add(Snapshot.NetworkId, SourceShard);
}

void UFlecsNetworkWorldSubsystem::RemoveRemoteEntity(const FFlecsNetworkId NetworkId)
{
	FFlecsEntityHandle* Entity = NetworkIdToEntityHandleMap.Find(NetworkId); 
	
	if (Entity && Entity->IsValid())
	{
		Entity->Destroy();
	}
	
	NetworkIdToEntityHandleMap.Remove(NetworkId);
	ClientSlotBindings.Remove(NetworkId.GetSlot());
	LastAppliedStateRevisions.Remove(NetworkId);
	EntitySourceShards.Remove(NetworkId);
	EntityPairFixups.RemoveAll([NetworkId](const FEntityPairFixup& Fixup) { return Fixup.Source == NetworkId; });
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
	for (int32 Index = EntityPairFixups.Num() - 1; Index >= 0; --Index)
	{
		const FEntityPairFixup& Fixup = EntityPairFixups[Index];
		
		FFlecsEntityHandle Source = FindEntity(Fixup.Source);
		FFlecsId PairId;
		
		if (!Source.IsValid() || !ResolveKeyToLocalId(Fixup.Key, PairId))
		{
			continue;
		}
		
		Source.Add(PairId);
		EntityPairFixups.RemoveAtSwap(Index, 1, EAllowShrinking::No);
	}
}

bool UFlecsNetworkWorldSubsystem::ResolveKeyToLocalId(const FFlecsReplicationKey& Key, FFlecsId& OutId) const
{
	const TSolidNotNull<const UFlecsWorld*> World = GetFlecsWorldChecked();
	const FFlecsComponentReplicationRegistry& Registry = FFlecsComponentReplicationRegistry::Get(World);
	const FFlecsComponentReplicationDescriptor* Storage = Registry.Find(Key.StorageSchema);
	
	if (!Storage || Storage->SchemaVersion != Key.StorageVersion)
	{
		return false;
	}
	
	if (Key.Kind == EFlecsReplicationKeyKind::Component)
	{
		OutId = Storage->LocalFlecsId;
		return true;
	}
	
	const FFlecsComponentReplicationDescriptor* Relationship = Registry.Find(Key.RelationshipSchema);
	
	if (!Relationship || Relationship->SchemaVersion != Key.RelationshipVersion)
	{
		return false;
	}

	FFlecsId Target;
	switch (Key.TargetKind)
	{
	case EFlecsReplicationPairTargetKind::Schema:
		if (const FFlecsComponentReplicationDescriptor* TargetDescriptor = Registry.Find(Key.TargetSchema))
		{
			if (TargetDescriptor->SchemaVersion != Key.TargetVersion)
			{
				return false;
			}
			
			Target = TargetDescriptor->LocalFlecsId;
		}
		break;
	case EFlecsReplicationPairTargetKind::StableSymbolValue:
	{
		const FFlecsEntityHandle StableTarget = World->LookupEntityBySymbol_Internal(Key.StableTargetIdentifier);
			
		if (StableTarget.IsValid())
		{
			Target = StableTarget.GetFlecsId();
		}
			
		break;
	}
		case EFlecsReplicationPairTargetKind::StablePathValue:
		{
			const FFlecsEntityHandle StableTarget = World->LookupEntity(Key.StableTargetIdentifier);
			
			if (StableTarget.IsValid())
			{
				Target = StableTarget.GetFlecsId();
			}
			
			break;
		}
	case EFlecsReplicationPairTargetKind::Entity:
	{
		const FFlecsEntityHandle EntityTarget = FindEntity(Key.EntityTarget);
			
		if (EntityTarget.IsValid())
		{
			Target = EntityTarget.GetFlecsId();
		}
			
		break;
	}
	default:
		break;
	}
	
	if (!Target.IsValid())
	{
		return false;
	}
	
	OutId = FFlecsId::MakePair(Relationship->LocalFlecsId, Target);
	return true;
}

bool UFlecsNetworkWorldSubsystem::ValidateLayout(const FFlecsReplicationLayoutDefinition& Layout, FString& OutError) const
{
	const FFlecsComponentReplicationRegistry& Registry = FFlecsComponentReplicationRegistry::Get(GetFlecsWorldChecked());
	
	for (const FFlecsReplicationKey& Key : Layout.Keys)
	{
		auto ValidateSchema = [&Registry, &OutError](const FFlecsReplicationSchemaId Schema, const uint32 Version,
			const TCHAR* Role)
		{
			const FFlecsComponentReplicationDescriptor* Descriptor = Registry.Find(Schema);
			
			if (!Descriptor || Descriptor->SchemaVersion != Version)
			{
				OutError = FString::Printf(TEXT("%s schema %s version %u is incompatible with the local schema set"),
					Role, *Schema.ToString(), Version);
				return false;
			}
			
			return true;
		};
		
		if (!ValidateSchema(Key.StorageSchema, Key.StorageVersion, TEXT("Storage")))
		{
			return false;
		}
		
		if (Key.Kind == EFlecsReplicationKeyKind::Pair
			&& !ValidateSchema(Key.RelationshipSchema, Key.RelationshipVersion, TEXT("Relationship")))
		{
			return false;
		}
		
		if (Key.TargetKind == EFlecsReplicationPairTargetKind::Schema
			&& !ValidateSchema(Key.TargetSchema, Key.TargetVersion, TEXT("Target")))
		{
			return false;
		}
		
		if (Key.TargetKind == EFlecsReplicationPairTargetKind::StableSymbolValue
			&& Key.StableTargetIdentifier.IsEmpty())
		{
			return false;
		}
		
		if (Key.TargetKind == EFlecsReplicationPairTargetKind::StablePathValue
			&& Key.StableTargetIdentifier.IsEmpty())
		{
			OutError = TEXT("Stable pair target path is missing");
			return false;
		}
	}
	
	return true;
}

void UFlecsNetworkWorldSubsystem::HandleWorldPreActorTick(UWorld* World, ELevelTick, float)
{
	if (World != GetWorld() || !IsFlecsWorldValid())
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
