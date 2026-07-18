// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsNetworkWorldSubsystem.h"

#include "Engine/World.h"
#include "Networking/FlecsNetRoleType.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

#include "Networking/FlecsNetworkingModuleSettings.h"
#include "Networking/FlecsNetworkingStats.h"
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
	ConnectionInterestContexts.Reset();
	LoggedInvalidInterestBindings.Reset();
	
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
		FReplicatedEntityState& State = EntityStates.FindOrAdd(*Existing);
		State.LastDirtyTime = GetReplicationTimeSeconds();
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
	FReplicatedEntityState& State = EntityStates.Add(NetworkId);
	State.LastDirtyTime = GetReplicationTimeSeconds();
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
		if (!State || State->bRouteInterestValid)
		{
			ReplicationTransport->RemoveEntity(
				State ? State->Route : FFlecsReplicationRouteDescriptor::Default(), NetworkId);
		}
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
	
	if LIKELY_IF(NetworkId && NetworkId->IsValid())
	{
		DirtyEntities.Add(*NetworkId);
		FReplicatedEntityState& State = EntityStates.FindOrAdd(*NetworkId);
		State.bAllPayloadDirty = true;
		State.LastDirtyTime = GetReplicationTimeSeconds();
		if (State.bDormant)
		{
			State.bDormant = false;
			if (ReplicationTransport)
			{
				ReplicationTransport->SetEntityDormancy(State.Route, *NetworkId, false);
			}
		}
	}
}

void UFlecsNetworkWorldSubsystem::MarkComponentDirty(const FFlecsEntityHandle& EntityHandle,
	const FFlecsId ComponentOrPairId)
{
	if UNLIKELY_IF(!HasAuthority() || !EntityHandle.IsValid() || !ComponentOrPairId.IsValid())
	{
		return;
	}

	const FFlecsNetworkId* NetworkId = EntityHandle.TryGet<FFlecsNetworkId>();
	if UNLIKELY_IF(!NetworkId || !NetworkId->IsValid())
	{
		return;
	}

	DirtyEntities.Add(*NetworkId);
	FReplicatedEntityState& State = EntityStates.FindOrAdd(*NetworkId);
	State.DirtyComponentIds.Add(ComponentOrPairId);
	State.LastDirtyTime = GetReplicationTimeSeconds();
	
	if (State.bDormant)
	{
		State.bDormant = false;
		if (ReplicationTransport)
		{
			ReplicationTransport->SetEntityDormancy(State.Route, *NetworkId, false);
		}
	}
}

void UFlecsNetworkWorldSubsystem::SetReplicationRouter(TUniquePtr<IFlecsReplicationRouter> InRouter)
{
	Router = InRouter ? MoveTemp(InRouter) : MakeUnique<FFlecsDefaultReplicationRouter>();
	for (const TPair<FFlecsNetworkId, FReplicatedEntityState>& Pair : EntityStates)
	{
		DirtyEntities.Add(Pair.Key);
	}
}

void UFlecsNetworkWorldSubsystem::ClearConnectionInterestContext(const uint32 ConnectionId)
{
	ConnectionInterestContexts.Remove(ConnectionId);
}

bool UFlecsNetworkWorldSubsystem::ValidateInterestBinding(
	const FFlecsReplicationInterestBinding& Binding, FString& OutError) const
{
	return FFlecsReplicationInterestPolicyRegistry::ValidateBinding(Binding, OutError);
}

bool UFlecsNetworkWorldSubsystem::IsRouteRelevant(const FFlecsReplicationRouteDescriptor& Route,
	const uint32 ConnectionId, const FFlecsReplicationConnectionView& View) const
{
	return IsInterestBindingRelevant(Route.Interest, Route.LogicalKey.Name, ConnectionId, View);
}

bool UFlecsNetworkWorldSubsystem::IsInterestBindingRelevant(
	const FFlecsReplicationInterestBinding& Binding, const FName RouteName,
	const uint32 ConnectionId, const FFlecsReplicationConnectionView& View) const
{
	FString Error;
	if (!ValidateInterestBinding(Binding, Error))
	{
		FFlecsReplicationRouteDescriptor DiagnosticRoute;
		DiagnosticRoute.LogicalKey = FFlecsReplicationRouteKey(RouteName);
		DiagnosticRoute.Interest = Binding;
		ReportInvalidInterestBinding(DiagnosticRoute, Error, TEXT("filter"));
		INC_DWORD_STAT(STAT_FlecsReplicationInvalidPolicyRejections);
		return false;
	}

	static const FFlecsReplicationConnectionInterestContext EmptyContext;
	const FFlecsReplicationConnectionInterestContext* Context = ConnectionInterestContexts.Find(ConnectionId);
	const IFlecsReplicationInterestPolicy* Policy =
		FFlecsReplicationInterestPolicyRegistry::FindPolicy(Binding.PolicyName);
	const FFlecsReplicationInterestEvaluationQuery Query{
		ConnectionId,
		Context ? *Context : EmptyContext,
		View
	};
	return Policy && Policy->IsInterested(Binding.Descriptor, Query);
}

void UFlecsNetworkWorldSubsystem::ReportInvalidInterestBinding(
	const FFlecsReplicationRouteDescriptor& Route, const FString& Error, const TCHAR* Source) const
{
	const FString DescriptorName = Route.Interest.Descriptor.GetScriptStruct()
		? Route.Interest.Descriptor.GetScriptStruct()->GetPathName() : TEXT("None");
	const FString LogKey = FString::Printf(TEXT("%s|%s|%s|%s"), *Route.LogicalKey.Name.ToString(),
		*Route.Interest.PolicyName.ToString(), *DescriptorName, *Error);
	if (LoggedInvalidInterestBindings.Contains(LogKey))
	{
		return;
	}

	LoggedInvalidInterestBindings.Add(LogKey);
	INC_DWORD_STAT(STAT_FlecsReplicationInvalidPolicyRejections);
	UE_LOG(LogFlecsCore, Warning,
		TEXT("Rejected %s interest binding for route '%s' (policy '%s', descriptor '%s'): %s"),
		Source, *Route.LogicalKey.Name.ToString(), *Route.Interest.PolicyName.ToString(),
		*DescriptorName, *Error);
}

void UFlecsNetworkWorldSubsystem::SetReplicationDormancy(const FFlecsEntityHandle& EntityHandle,
	const EFlecsReplicationDormancyMode Mode)
{
	if (!HasAuthority() || !EntityHandle.IsValid())
	{
		return;
	}
	const FFlecsNetworkId* NetworkId = EntityHandle.TryGet<FFlecsNetworkId>();
	if (!NetworkId || !NetworkId->IsValid())
	{
		return;
	}
	FReplicatedEntityState& State = EntityStates.FindOrAdd(*NetworkId);
	State.DormancyMode = Mode;
	if (Mode == EFlecsReplicationDormancyMode::ForceAwake && State.bDormant)
	{
		State.bDormant = false;
		if (ReplicationTransport)
		{
			ReplicationTransport->SetEntityDormancy(State.Route, *NetworkId, false);
		}
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
			.each([WeakThis](flecs::iter& Iterator, const size_t Index)
		{
			if (UFlecsNetworkWorldSubsystem* Subsystem = WeakThis.Get())
			{
				Subsystem->MarkComponentDirty(FFlecsEntityHandle(Iterator.entity(Index)),
					FFlecsId(Iterator.event_id()));
			}
		});
		
		return Observer;
	};
	
	DirtyObservers.Add(Install(Descriptor.LocalFlecsId.GetId()));
	DirtyObservers.Add(Install(FFlecsId::MakePair(Descriptor.LocalFlecsId.GetId(), flecs::Wildcard)));
	DirtyObservers.Add(Install(FFlecsId::MakePair(flecs::Wildcard, Descriptor.LocalFlecsId.GetId())));
}

void UFlecsNetworkWorldSubsystem::GatherDirtyEntities()
{
	const TSolidNotNull<UFlecsWorld*> World = GetFlecsWorldChecked();
	FFlecsComponentReplicationRegistry& ComponentRegistry = FFlecsComponentReplicationRegistry::Get(World);
	const double Now = GetReplicationTimeSeconds();
	const TSolidNotNull<const UFlecsNetworkingModuleSettings*> Settings = GetNetworkingModuleSettings();
	
	auto ResolveRoute = [this, Settings](const FFlecsEntityHandle& Entity, bool& bOutInterestValid)
	{
		FFlecsReplicationRouteDescriptor Route = Router->Route(Entity);
		
		if (Route == FFlecsReplicationRouteDescriptor::Default())
		{
			Route.PollFrequency = Settings->DefaultShardPollFrequency;
			Route.StaticPriority = Settings->DefaultShardStaticPriority;
		}

		FString InterestError;
		bOutInterestValid = ValidateInterestBinding(Route.Interest, InterestError);
		if (!bOutInterestValid)
		{
			ReportInvalidInterestBinding(Route, InterestError, TEXT("authority"));
		}
		
		return Route;
	};

	// Route selection is intentionally global. A non-replicated routing component
	// or custom router can move a clean entity without needing a payload observer.
	for (TPair<FFlecsNetworkId, FReplicatedEntityState>& Pair : EntityStates)
	{
		const FFlecsEntityHandle* Entity = NetworkIdToEntityHandleMap.Find(Pair.Key);
		bool bInterestValid = false;
		const FFlecsReplicationRouteDescriptor Route = Entity && Entity->IsValid()
			? ResolveRoute(*Entity, bInterestValid) : Pair.Value.Route;
		if (Entity && Entity->IsValid()
			&& ((bInterestValid && Route != Pair.Value.Route)
				|| bInterestValid != Pair.Value.bRouteInterestValid))
		{
			if (Pair.Value.bDormant)
			{
				Pair.Value.bDormant = false;
				if (ReplicationTransport)
				{
					ReplicationTransport->SetEntityDormancy(Pair.Value.Route, Pair.Key, false);
				}
			}
			
			Pair.Value.bNeedsFullUpdate = true;
			Pair.Value.bAllPayloadDirty = true;
			Pair.Value.LastDirtyTime = Now;
			DirtyEntities.Add(Pair.Key);
		}
	}

	TArray<FFlecsNetworkId> Dirty = DirtyEntities.Array();
	DirtyEntities.Reset();

	for (const FFlecsNetworkId NetworkId : Dirty)
	{
		const FFlecsEntityHandle* EntityPtr = NetworkIdToEntityHandleMap.Find(NetworkId);
		
		if UNLIKELY_IF(!EntityPtr || !EntityPtr->IsValid())
		{
			continue;
		}
		
		const FFlecsEntityHandle Entity = *EntityPtr;
		FReplicatedEntityState& State = EntityStates.FindOrAdd(NetworkId);
		const FFlecsReplicationRouteDescriptor PreviousRoute = State.Route;
		const bool bPreviousInterestValid = State.bRouteInterestValid;
		bool bInterestValid = false;
		const FFlecsReplicationRouteDescriptor Route = ResolveRoute(Entity, bInterestValid);
		const bool bRouteChanged = State.Route != Route;

		if (!bInterestValid)
		{
			if (State.bRouteInterestValid && State.StateRevision > 0 && ReplicationTransport)
			{
				ReplicationTransport->RemoveEntity(PreviousRoute, NetworkId);
				PublishedLayoutRoutes.RemoveAll(
					[&PreviousRoute](const FPublishedLayoutKey& Key)
					{
						return Key.Route == PreviousRoute;
					});
			}
			State.bRouteInterestValid = false;
			State.bNeedsFullUpdate = true;
			State.bAllPayloadDirty = true;
			continue;
		}

		bool bLayoutCreated = false;
		FString Error;
		const FFlecsReplicationLayoutDefinition* Layout = LayoutRegistry.BuildForEntity(
			World, Entity, bLayoutCreated, Error);
		
		if UNLIKELY_IF(!Layout)
		{
			UE_LOG(LogFlecsCore, Error, TEXT("Failed to gather entity %llu: %s"), NetworkId.GetValue(), *Error);
			continue;
		}

		const bool bLayoutChanged = State.LayoutId != Layout->LayoutId;

		if (bLayoutChanged)
		{
			State.LayoutId = Layout->LayoutId;
			++State.CompositionRevision;
			State.bNeedsFullUpdate = true;
			State.bAllPayloadDirty = true;
			State.CanonicalValues.Reset();
			State.LastSentCanonicalValues.Reset();
			State.LastSentEncodedValues.Reset();
			State.PendingEncodedPayloads.Reset();
		}

		if (bRouteChanged)
		{
			State.bNeedsFullUpdate = true;
		}
		State.Route = Route;
		State.bRouteInterestValid = true;

		const FPublishedLayoutKey PublishedKey{ Route, Layout->LayoutId };
		
		if (!PublishedLayoutRoutes.Contains(PublishedKey) && ReplicationTransport)
		{
			ReplicationTransport->PublishLayout(Route, *Layout);
			PublishedLayoutRoutes.Add(PublishedKey);
		}

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

			const bool bCapture = State.bAllPayloadDirty || State.bNeedsFullUpdate
				|| State.DirtyComponentIds.Contains(LocalId);
			if (!bCapture)
			{
				continue;
			}
			
			const FFlecsComponentReplicationDescriptor* Descriptor = ComponentRegistry.Find(Key.StorageSchema);
			const void* Value = Entity.TryGet(LocalId);
			
			if (!Descriptor || !Value)
			{
				continue;
			}
			
			TArray<uint8> CanonicalBytes;
			FMemoryWriter CanonicalWriter(CanonicalBytes, true);
			if UNLIKELY_IF(!Descriptor->Serialize(CanonicalWriter, const_cast<void*>(Value)) || CanonicalWriter.IsError())
			{
				UE_LOG(LogFlecsCore, Error, TEXT("Failed to serialize schema '%s' for entity %llu"),
					*Descriptor->StableName, NetworkId.GetValue());
				continue;
			}

			const uint16 EncodedKeyIndex = static_cast<uint16>(KeyIndex);
			State.CanonicalValues.Add(EncodedKeyIndex, CanonicalBytes);
			const TArray<uint8>* LastSent = State.LastSentCanonicalValues.Find(EncodedKeyIndex);
			if (LastSent && *LastSent == CanonicalBytes && !State.bNeedsFullUpdate)
			{
				State.PendingEncodedPayloads.Remove(EncodedKeyIndex);
				continue;
			}

			TArray<uint8> EncodedBytes;
			FMemoryWriter EncodedWriter(EncodedBytes, true);
			if UNLIKELY_IF(!Descriptor->QuantizeAndSerialize(EncodedWriter, const_cast<void*>(Value))
				|| EncodedWriter.IsError())
			{
				UE_LOG(LogFlecsCore, Error, TEXT("Failed to quantize schema '%s' for entity %llu"),
					*Descriptor->StableName, NetworkId.GetValue());
				continue;
			}
			if (const TArray<uint8>* LastEncoded = State.LastSentEncodedValues.Find(EncodedKeyIndex);
				LastEncoded && *LastEncoded == EncodedBytes && !State.bNeedsFullUpdate)
			{
				State.LastSentCanonicalValues.Add(EncodedKeyIndex, CanonicalBytes);
				State.PendingEncodedPayloads.Remove(EncodedKeyIndex);
				continue;
			}
			State.PendingEncodedPayloads.Add(EncodedKeyIndex, MoveTemp(EncodedBytes));
		}

		State.DirtyComponentIds.Reset();
		State.bAllPayloadDirty = false;

		if (State.bNeedsFullUpdate && ReplicationTransport)
		{
			FFlecsReplicatedEntityUpdate Update;
			Update.NetworkId = NetworkId;
			Update.StateRevision = State.StateRevision + 1;
			Update.CompositionRevision = State.CompositionRevision;
			Update.Kind = EFlecsReplicatedEntityUpdateKind::Full;
			Update.LayoutId = Layout->LayoutId;
			Update.Route = Route;

			for (int32 KeyIndex = 0; KeyIndex < Layout->Keys.Num(); ++KeyIndex)
			{
				const FFlecsReplicationKey& Key = Layout->Keys[KeyIndex];
				
				if (!Key.bHasPayload)
				{
					continue;
				}
				
				const uint16 EncodedKeyIndex = static_cast<uint16>(KeyIndex);
				const TArray<uint8>* Encoded = State.PendingEncodedPayloads.Find(EncodedKeyIndex);
				
				if (!Encoded)
				{
					FFlecsId LocalId;
					const FFlecsComponentReplicationDescriptor* Descriptor = ComponentRegistry.Find(Key.StorageSchema);
					const void* Value = ResolveKeyToLocalId(Key, LocalId) ? Entity.TryGet(LocalId) : nullptr;
					
					if (!Descriptor || !Value)
					{
						continue;
					}
					
					TArray<uint8> EncodedValue;
					FMemoryWriter Writer(EncodedValue, true);
					
					if (!Descriptor->QuantizeAndSerialize(Writer, const_cast<void*>(Value)) || Writer.IsError())
					{
						continue;
					}
					
					Encoded = &State.PendingEncodedPayloads.Add(EncodedKeyIndex, MoveTemp(EncodedValue));
				}
				
				FFlecsReplicatedValue& Serialized = Update.Values.AddDefaulted_GetRef();
				Serialized.KeyIndex = EncodedKeyIndex;
				Serialized.Bytes = *Encoded;
				Update.SetKeyChanged(EncodedKeyIndex);
			}

			if (bPreviousInterestValid && bRouteChanged && PreviousRoute != Route
				&& State.StateRevision > 0)
			{
				ReplicationTransport->MigrateEntity(PreviousRoute, Route, *Layout, Update);
				INC_DWORD_STAT(STAT_FlecsReplicationMigrations);
			}
			else
			{
				ReplicationTransport->PublishEntity(Route, Update);
			}

			State.StateRevision = Update.StateRevision;
			
			INC_DWORD_STAT_BY(STAT_FlecsReplicationFullBytes, Update.GetPayloadByteCount());
			INC_DWORD_STAT_BY(STAT_FlecsReplicationQuantizedBytes, Update.GetPayloadByteCount());
			
			for (const TPair<uint16, TArray<uint8>>& Pair : State.CanonicalValues)
			{
				State.LastSentCanonicalValues.Add(Pair.Key, Pair.Value);
			}
			
			for (const FFlecsReplicatedValue& Value : Update.Values)
			{
				State.LastSentEncodedValues.Add(Value.KeyIndex, Value.Bytes);
			}
			
			State.PendingEncodedPayloads.Reset();
			for (const FFlecsReplicationKey& Key : Layout->Keys)
			{
				if (Key.bHasPayload)
				{
					State.LastSendTimes.Add(Key.StorageSchema, Now);
				}
			}
			
			State.bNeedsFullUpdate = false;
		}
	}

	struct FScheduledKey
	{
		FFlecsNetworkId NetworkId;
		uint16 KeyIndex = 0;
		FFlecsReplicationSchemaId Schema;
		double Score = 0.0;
		double AgeSeconds = 0.0;
		uint32 Bytes = 0;
	};

	TArray<FScheduledKey> Candidates;
	for (TPair<FFlecsNetworkId, FReplicatedEntityState>& Pair : EntityStates)
	{
		FReplicatedEntityState& State = Pair.Value;
		if (State.bNeedsFullUpdate || State.PendingEncodedPayloads.IsEmpty())
		{
			continue;
		}
		const FFlecsReplicationLayoutDefinition* Layout = LayoutRegistry.Find(State.LayoutId);
		if (!Layout)
		{
			continue;
		}
		for (const TPair<uint16, TArray<uint8>>& Pending : State.PendingEncodedPayloads)
		{
			if (!Layout->Keys.IsValidIndex(Pending.Key))
			{
				continue;
			}
			const FFlecsReplicationKey& Key = Layout->Keys[Pending.Key];
			const FFlecsComponentReplicationDescriptor* Descriptor = ComponentRegistry.Find(Key.StorageSchema);
			if (!Descriptor)
			{
				continue;
			}
			const double LastSendTime = State.LastSendTimes.FindRef(Key.StorageSchema);
			const double Age = FMath::Max(0.0, Now - LastSendTime);
			if (Descriptor->UpdateFrequencyHz > 0.0f && Age < 1.0 / Descriptor->UpdateFrequencyHz)
			{
				continue;
			}
			FScheduledKey& Candidate = Candidates.AddDefaulted_GetRef();
			Candidate.NetworkId = Pair.Key;
			Candidate.KeyIndex = Pending.Key;
			Candidate.Schema = Key.StorageSchema;
			Candidate.Bytes = Pending.Value.Num();
			Candidate.AgeSeconds = Age;
			Candidate.Score = FMath::Max(0.001f, State.Route.SchedulerWeight)
				* FMath::Max(0.001f, Descriptor->ReplicationPriority) * FMath::Max(0.001, Age);
		}
	}

	Candidates.Sort([](const FScheduledKey& A, const FScheduledKey& B)
	{
		if (A.Score != B.Score)
		{
			return A.Score > B.Score;
		}
		if (A.NetworkId != B.NetworkId)
		{
			return A.NetworkId.GetValue() < B.NetworkId.GetValue();
		}
		return A.KeyIndex < B.KeyIndex;
	});

	uint32 Budget = Settings->MaxPayloadBytesPerTick;
#if WITH_AUTOMATION_TESTS || WITH_EDITOR
	if (TestingPayloadBudget.IsSet())
	{
		Budget = TestingPayloadBudget.GetValue();
	}
#endif
	uint32 UsedBytes = 0;
	uint32 SelectedKeyCount = 0;
	TMap<FFlecsNetworkId, TArray<FScheduledKey>> Selected;
	for (const FScheduledKey& Candidate : Candidates)
	{
		if (Budget != 0 && UsedBytes != 0 && UsedBytes + Candidate.Bytes > Budget)
		{
			continue;
		}
		Selected.FindOrAdd(Candidate.NetworkId).Add(Candidate);
		UsedBytes += Candidate.Bytes;
		++SelectedKeyCount;
	}

	if (ReplicationTransport)
	{
		for (TPair<FFlecsNetworkId, TArray<FScheduledKey>>& Pair : Selected)
		{
			FReplicatedEntityState* State = EntityStates.Find(Pair.Key);
			if (!State)
			{
				continue;
			}
			
			Pair.Value.Sort([](const FScheduledKey& A, const FScheduledKey& B)
			{
				return A.KeyIndex < B.KeyIndex;
			});
			
			FFlecsReplicatedEntityUpdate Update;
			Update.NetworkId = Pair.Key;
			Update.StateRevision = State->StateRevision + 1;
			Update.CompositionRevision = State->CompositionRevision;
			Update.Kind = EFlecsReplicatedEntityUpdateKind::Delta;
			Update.LayoutId = State->LayoutId;
			Update.Route = State->Route;
			
			for (const FScheduledKey& Scheduled : Pair.Value)
			{
				const TArray<uint8>* Encoded = State->PendingEncodedPayloads.Find(Scheduled.KeyIndex);
				if (!Encoded)
				{
					continue;
				}
				FFlecsReplicatedValue& Value = Update.Values.AddDefaulted_GetRef();
				Value.KeyIndex = Scheduled.KeyIndex;
				Value.Bytes = *Encoded;
				Update.SetKeyChanged(Scheduled.KeyIndex);
			}
			
			if (Update.Values.IsEmpty())
			{
				continue;
			}
			
			ReplicationTransport->PublishEntity(State->Route, Update);
			State->StateRevision = Update.StateRevision;
			INC_DWORD_STAT_BY(STAT_FlecsReplicationDeltaBytes, Update.GetPayloadByteCount());
			INC_DWORD_STAT_BY(STAT_FlecsReplicationQuantizedBytes, Update.GetPayloadByteCount());
			for (const FScheduledKey& Scheduled : Pair.Value)
			{
				if (const TArray<uint8>* Canonical = State->CanonicalValues.Find(Scheduled.KeyIndex))
				{
					State->LastSentCanonicalValues.Add(Scheduled.KeyIndex, *Canonical);
				}
				
				if (const TArray<uint8>* Encoded = State->PendingEncodedPayloads.Find(Scheduled.KeyIndex))
				{
					State->LastSentEncodedValues.Add(Scheduled.KeyIndex, *Encoded);
				}
				
				State->PendingEncodedPayloads.Remove(Scheduled.KeyIndex);
				State->LastSendTimes.Add(Scheduled.Schema, Now);
			}
		}
	}

	for (TPair<FFlecsNetworkId, FReplicatedEntityState>& Pair : EntityStates)
	{
		FReplicatedEntityState& State = Pair.Value;
		const bool bClean = !State.bNeedsFullUpdate && State.PendingEncodedPayloads.IsEmpty()
			&& !DirtyEntities.Contains(Pair.Key);
		
		const bool bShouldDorm = State.DormancyMode == EFlecsReplicationDormancyMode::DormantUntilDirty
			? bClean
			: State.DormancyMode == EFlecsReplicationDormancyMode::Automatic && bClean
				&& Now - State.LastDirtyTime >= Settings->AutomaticDormancyDelaySeconds;
		if (bShouldDorm != State.bDormant && State.DormancyMode != EFlecsReplicationDormancyMode::ForceAwake)
		{
			State.bDormant = bShouldDorm;
			if (ReplicationTransport)
			{
				ReplicationTransport->SetEntityDormancy(State.Route, Pair.Key, bShouldDorm);
			}
		}
	}

	uint32 PendingKeyCount = 0;
	uint32 DormantEntityCount = 0;
	double MaximumStarvationAge = 0.0;
	TSet<FName> Routes;
	for (const TPair<FFlecsNetworkId, FReplicatedEntityState>& Pair : EntityStates)
	{
		PendingKeyCount += Pair.Value.PendingEncodedPayloads.Num();
		DormantEntityCount += Pair.Value.bDormant ? 1u : 0u;
		Routes.Add(Pair.Value.Route.LogicalKey.Name);
	}
	
	for (const FScheduledKey& Candidate : Candidates)
	{
		MaximumStarvationAge = FMath::Max(MaximumStarvationAge, Candidate.AgeSeconds);
	}
	
	SET_DWORD_STAT(STAT_FlecsReplicationPendingKeys, PendingKeyCount);
	SET_DWORD_STAT(STAT_FlecsReplicationDeferredKeys, PendingKeyCount - SelectedKeyCount);
	SET_DWORD_STAT(STAT_FlecsReplicationActiveEntities, EntityStates.Num() - DormantEntityCount);
	SET_DWORD_STAT(STAT_FlecsReplicationDormantEntities, DormantEntityCount);
	SET_DWORD_STAT(STAT_FlecsReplicationRoutes, Routes.Num());
	SET_DWORD_STAT(STAT_FlecsReplicationStarvationAgeMs,
		static_cast<uint32>(MaximumStarvationAge * 1000.0));
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
			RemoteLayoutSources.FindOrAdd(Record.Layout.LayoutId).Add(Record.SourceShard);
				
			if (TArray<TPair<FGuid, FFlecsReplicatedEntityUpdate>>* Deferred = DeferredUpdates.Find(Record.Layout.LayoutId))
			{
				TArray<TPair<FGuid, FFlecsReplicatedEntityUpdate>> Pending = MoveTemp(*Deferred);
				DeferredUpdates.Remove(Record.Layout.LayoutId);
				
				for (const auto& Item : Pending)
				{
					ApplyUpdate(Item.Key, Item.Value);
				}
			}
				
			break;
		}
		case EFlecsReplicationInboxRecordType::RemoveLayout:
			if (TSet<FGuid>* Sources = RemoteLayoutSources.Find(Record.Layout.LayoutId))
			{
				Sources->Remove(Record.SourceShard);
				if (Sources->IsEmpty())
				{
					RemoteLayoutSources.Remove(Record.Layout.LayoutId);
				}
			}
			TryReclaimRemoteLayout(Record.Layout.LayoutId);
			break;
		case EFlecsReplicationInboxRecordType::UpsertEntity:
			ApplyUpdate(Record.SourceShard, Record.Update);
			break;
		case EFlecsReplicationInboxRecordType::UpdateChunk:
		{
			const FFlecsReplicationLayoutId ChunkLayoutId = Record.Chunk.LayoutId;
			TOptional<FFlecsReplicatedEntityUpdate> CompleteUpdate;
			FString Error;
			if (!UpdateReassembler.Accept(Record.SourceShard, Record.Chunk, CompleteUpdate, Error))
			{
				if (ReplicationTransport)
				{
					ReplicationTransport->HandleProtocolError(Error);
				}
			}
			else if (CompleteUpdate.IsSet())
			{
				ApplyUpdate(Record.SourceShard, CompleteUpdate.GetValue());
			}
			TryReclaimRemoteLayout(ChunkLayoutId);
			break;
		}
		case EFlecsReplicationInboxRecordType::RemoveEntity:
			RemoveRemoteEntity(Record.NetworkId, &Record.SourceShard);
			break;
		case EFlecsReplicationInboxRecordType::DetachShard:
			DetachRemoteShard(Record.SourceShard);
			break;
		}
	}
	
	RetryEntityPairFixups();
}

void UFlecsNetworkWorldSubsystem::ApplyUpdate(const FGuid& SourceShard,
	const FFlecsReplicatedEntityUpdate& Update)
{
	if (!SourceShard.IsValid() || !Update.NetworkId.IsValid() || !Update.LayoutId.IsValid()
		|| Update.StateRevision == 0 || Update.CompositionRevision == 0
		|| (Update.Kind != EFlecsReplicatedEntityUpdateKind::Full
			&& Update.Kind != EFlecsReplicatedEntityUpdateKind::Delta))
	{
		if (ReplicationTransport)
		{
			ReplicationTransport->HandleProtocolError(TEXT("Received invalid Flecs replication update metadata"));
		}
		return;
	}

	FString InterestError;
	if (!ValidateInterestBinding(Update.Route.Interest, InterestError))
	{
		ReportInvalidInterestBinding(Update.Route, InterestError, TEXT("received"));
		if (ReplicationTransport)
		{
			ReplicationTransport->HandleProtocolError(FString::Printf(
				TEXT("Received invalid Flecs replication interest binding: %s"), *InterestError));
		}
		return;
	}

	const FFlecsReplicationLayoutDefinition* Layout = LayoutRegistry.Find(Update.LayoutId);
	
	if (!Layout)
	{
		DeferredUpdates.FindOrAdd(Update.LayoutId).Emplace(SourceShard, Update);
		return;
	}
	
	if (const uint32* Revision = LastAppliedStateRevisions.Find(Update.NetworkId);
		Revision && *Revision >= Update.StateRevision)
	{
		return;
	}

	const int32 RequiredMaskWords = FMath::DivideAndRoundUp(Layout->Keys.Num(),
		FFlecsReplicatedEntityUpdate::KeyMaskWordBits);
	if (Update.ChangedKeyMask.Num() > RequiredMaskWords)
	{
		if (ReplicationTransport)
		{
			ReplicationTransport->HandleProtocolError(TEXT("Received oversized Flecs replication update mask"));
		}
		return;
	}
	if (Update.Kind == EFlecsReplicatedEntityUpdateKind::Delta && Update.Values.IsEmpty())
	{
		if (ReplicationTransport)
		{
			ReplicationTransport->HandleProtocolError(TEXT("Received empty Flecs replication delta"));
		}
		return;
	}
	if (Update.ChangedKeyMask.Num() == RequiredMaskWords && !Update.ChangedKeyMask.IsEmpty()
		&& Layout->Keys.Num() % FFlecsReplicatedEntityUpdate::KeyMaskWordBits != 0)
	{
		const int32 ValidBits = Layout->Keys.Num() % FFlecsReplicatedEntityUpdate::KeyMaskWordBits;
		const uint64 ValidMask = (uint64(1) << ValidBits) - 1;
		if ((Update.ChangedKeyMask.Last() & ~ValidMask) != 0)
		{
			if (ReplicationTransport)
			{
				ReplicationTransport->HandleProtocolError(TEXT("Received out-of-range Flecs replication update mask"));
			}
			return;
		}
	}

	TSet<uint16> ValueKeys;
	for (const FFlecsReplicatedValue& Value : Update.Values)
	{
		if (!Layout->Keys.IsValidIndex(Value.KeyIndex) || !Update.IsKeyChanged(Value.KeyIndex)
			|| !Layout->Keys[Value.KeyIndex].bHasPayload || ValueKeys.Contains(Value.KeyIndex))
		{
			if (ReplicationTransport)
			{
				ReplicationTransport->HandleProtocolError(TEXT("Received invalid Flecs replication update mask/value"));
			}
			return;
		}
		ValueKeys.Add(Value.KeyIndex);
	}

	for (int32 KeyIndex = 0; KeyIndex < Layout->Keys.Num(); ++KeyIndex)
	{
		const bool bChanged = Update.IsKeyChanged(static_cast<uint16>(KeyIndex));
		const bool bHasValue = ValueKeys.Contains(static_cast<uint16>(KeyIndex));
		const bool bPayloadKey = Layout->Keys[KeyIndex].bHasPayload;
		if (bChanged != bHasValue || (Update.Kind == EFlecsReplicatedEntityUpdateKind::Full
			&& bPayloadKey && !bChanged))
		{
			if (ReplicationTransport)
			{
				ReplicationTransport->HandleProtocolError(TEXT("Received incomplete Flecs replication update"));
			}
			return;
		}
	}

	FFlecsReplicatedEntityUpdate Materialized;
	if (Update.Kind == EFlecsReplicatedEntityUpdateKind::Full)
	{
		Materialized = Update;
	}
	else
	{
		const FFlecsReplicatedEntityUpdate* Previous = MaterializedRemoteUpdates.Find(Update.NetworkId);
		if (!Previous || Previous->LayoutId != Update.LayoutId
			|| Previous->CompositionRevision != Update.CompositionRevision)
		{
			return;
		}
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
	MaterializedRemoteUpdates.Add(Update.NetworkId, Materialized);
	ApplyMaterializedUpdate(SourceShard, Materialized);
}

void UFlecsNetworkWorldSubsystem::ApplyMaterializedUpdate(const FGuid& SourceShard,
	const FFlecsReplicatedEntityUpdate& Update)
{
	const FFlecsReplicationLayoutDefinition* Layout = LayoutRegistry.Find(Update.LayoutId);
	if (!Layout)
	{
		return;
	}
	
	const FFlecsNetworkId* Bound = ClientSlotBindings.Find(Update.NetworkId.GetSlot());

	if (Bound && *Bound != Update.NetworkId)
	{
		if (Bound->GetValue() > Update.NetworkId.GetValue())
		{
			return;
		}
		
		RemoveRemoteEntity(*Bound);
	}

	// Accepted snapshots are complete replacements. Any unresolved pairs from
	// an older state revision no longer describe this source's current state.
	EntityPairFixups.RemoveAll([&Update](const FEntityPairFixup& Fixup)
	{
		return Fixup.Source == Update.NetworkId;
	});

	const TSolidNotNull<UFlecsWorld*> World = GetFlecsWorldChecked();
	FFlecsEntityHandle Entity = FindEntity(Update.NetworkId);
	
	if (!Entity.IsValid())
	{
		Entity = World->CreateEntity();
		Entity.Set<FFlecsNetworkId>(Update.NetworkId);
		Entity.Add<FFlecsReplicatedEntityComponent>();
		Entity.Add<EFlecsNetRoleType>(EFlecsNetRoleType::SimulatedProxy);
		
		NetworkIdToEntityHandleMap.Add(Update.NetworkId, Entity);
		ClientSlotBindings.Add(Update.NetworkId.GetSlot(), Update.NetworkId);
	}

	TSet<FFlecsId> DesiredIds;
	for (int32 KeyIndex = 0; KeyIndex < Layout->Keys.Num(); ++KeyIndex)
	{
		const FFlecsReplicationKey& Key = Layout->Keys[KeyIndex];
		FFlecsId LocalId;
		
		if (ResolveKeyToLocalId(Key, LocalId))
		{
			DesiredIds.Add(LocalId);
		}
		else if (Key.TargetKind == EFlecsReplicationPairTargetKind::Entity)
		{
			TOptional<TArray<uint8>> Payload;
			const FFlecsReplicatedValue* Value = Update.Values.FindByPredicate(
				[KeyIndex](const FFlecsReplicatedValue& Candidate)
				{
					return Candidate.KeyIndex == KeyIndex;
				});

			if (Value)
			{
				Payload = Value->Bytes;
			}

			EntityPairFixups.Add({ Update.NetworkId, Key.EntityTarget, Key,
				Update.StateRevision, MoveTemp(Payload) });
		}
	}

	const FFlecsReplicationLayoutDefinition* PreviousLayout = nullptr;
	if (const FFlecsReplicationLayoutId* PreviousLayoutId = LastAppliedLayoutIds.Find(Update.NetworkId))
	{
		PreviousLayout = LayoutRegistry.Find(*PreviousLayoutId);
	}
	World->Defer([&]()
	{
		TArray<FFlecsId> ToRemove;

		if (PreviousLayout)
		{
			for (const FFlecsReplicationKey& PreviousKey : PreviousLayout->Keys)
			{
				FFlecsId PreviousId;
				if (ResolveKeyToLocalId(PreviousKey, PreviousId) && !DesiredIds.Contains(PreviousId))
				{
					ToRemove.Add(PreviousId);
				}
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

		for (const FFlecsReplicatedValue& Value : Update.Values)
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

	LastAppliedStateRevisions.Add(Update.NetworkId, Update.StateRevision);
	LastAppliedLayoutIds.Add(Update.NetworkId, Update.LayoutId);
	EntitySourceShards.Add(Update.NetworkId, SourceShard);
}

void UFlecsNetworkWorldSubsystem::RemoveRemoteEntity(const FFlecsNetworkId NetworkId,
	const FGuid* ExpectedSource)
{
	if (ExpectedSource)
	{
		UpdateReassembler.RemoveEntity(*ExpectedSource, NetworkId);
		TArray<FFlecsReplicationLayoutId> EmptyDeferredLayouts;
		for (TPair<FFlecsReplicationLayoutId, TArray<TPair<FGuid, FFlecsReplicatedEntityUpdate>>>& Pair
			: DeferredUpdates)
		{
			Pair.Value.RemoveAll([ExpectedSource, NetworkId](const TPair<FGuid, FFlecsReplicatedEntityUpdate>& Deferred)
			{
				return Deferred.Key == *ExpectedSource && Deferred.Value.NetworkId == NetworkId;
			});
			if (Pair.Value.IsEmpty())
			{
				EmptyDeferredLayouts.Add(Pair.Key);
			}
		}
		for (const FFlecsReplicationLayoutId LayoutId : EmptyDeferredLayouts)
		{
			DeferredUpdates.Remove(LayoutId);
			TryReclaimRemoteLayout(LayoutId);
		}

		const FGuid* CurrentSource = EntitySourceShards.Find(NetworkId);
		if (CurrentSource && *CurrentSource != *ExpectedSource)
		{
			return;
		}
		if (!CurrentSource)
		{
			EntityPairFixups.RemoveAll([NetworkId](const FEntityPairFixup& Fixup)
			{
				return Fixup.Source == NetworkId || Fixup.Target == NetworkId;
			});
			return;
		}
	}

	const FFlecsReplicationLayoutId RemovedLayout = LastAppliedLayoutIds.FindRef(NetworkId);
	const FFlecsEntityHandle* Entity = NetworkIdToEntityHandleMap.Find(NetworkId); 
	const bool bHadEntity = Entity && Entity->IsValid();
	if (bHadEntity)
	{
		Entity->Destroy();
		NetworkIdToEntityHandleMap.Remove(NetworkId);
		if (ClientSlotBindings.FindRef(NetworkId.GetSlot()) == NetworkId)
		{
			ClientSlotBindings.Remove(NetworkId.GetSlot());
		}
		LastAppliedStateRevisions.Remove(NetworkId);
		LastAppliedLayoutIds.Remove(NetworkId);
		EntitySourceShards.Remove(NetworkId);
		MaterializedRemoteUpdates.Remove(NetworkId);
	}

	EntityPairFixups.RemoveAll([NetworkId](const FEntityPairFixup& Fixup)
	{
		return Fixup.Source == NetworkId || Fixup.Target == NetworkId;
	});
	if (RemovedLayout.IsValid())
	{
		TryReclaimRemoteLayout(RemovedLayout);
	}
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
		RemoveRemoteEntity(Id, &SourceShard);
	}
	UpdateReassembler.RemoveSource(SourceShard);
	TArray<FFlecsReplicationLayoutId> EmptyDeferredLayouts;
	for (TPair<FFlecsReplicationLayoutId, TArray<TPair<FGuid, FFlecsReplicatedEntityUpdate>>>& Pair
		: DeferredUpdates)
	{
		Pair.Value.RemoveAll([SourceShard](const TPair<FGuid, FFlecsReplicatedEntityUpdate>& Deferred)
		{
			return Deferred.Key == SourceShard;
		});
		if (Pair.Value.IsEmpty())
		{
			EmptyDeferredLayouts.Add(Pair.Key);
		}
	}
	for (const FFlecsReplicationLayoutId LayoutId : EmptyDeferredLayouts)
	{
		DeferredUpdates.Remove(LayoutId);
		TryReclaimRemoteLayout(LayoutId);
	}
	for (TPair<FFlecsReplicationLayoutId, TSet<FGuid>>& Pair : RemoteLayoutSources)
	{
		Pair.Value.Remove(SourceShard);
	}
	TArray<FFlecsReplicationLayoutId> Layouts;
	RemoteLayoutSources.GetKeys(Layouts);
	for (const FFlecsReplicationLayoutId LayoutId : Layouts)
	{
		if (RemoteLayoutSources.FindChecked(LayoutId).IsEmpty())
		{
			RemoteLayoutSources.Remove(LayoutId);
		}
		TryReclaimRemoteLayout(LayoutId);
	}
}

void UFlecsNetworkWorldSubsystem::TryReclaimRemoteLayout(const FFlecsReplicationLayoutId LayoutId)
{
	if (RemoteLayoutSources.Contains(LayoutId) || DeferredUpdates.Contains(LayoutId)
		|| UpdateReassembler.ReferencesLayout(LayoutId))
	{
		return;
	}
	for (const TPair<FFlecsNetworkId, FFlecsReplicationLayoutId>& Pair : LastAppliedLayoutIds)
	{
		if (Pair.Value == LayoutId)
		{
			return;
		}
	}
	for (const TPair<FFlecsNetworkId, FFlecsReplicatedEntityUpdate>& Pair : MaterializedRemoteUpdates)
	{
		if (Pair.Value.LayoutId == LayoutId)
		{
			return;
		}
	}
	LayoutRegistry.RemoveDefinition(LayoutId);
	INC_DWORD_STAT(STAT_FlecsReplicationReclaimedLayouts);
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

	const FFlecsComponentReplicationRegistry& Registry =
		FFlecsComponentReplicationRegistry::Get(GetFlecsWorldChecked());
	const FFlecsComponentReplicationDescriptor* Descriptor = Registry.Find(Key.StorageSchema);

	if (!Descriptor || Descriptor->bIsTag)
	{
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

bool UFlecsNetworkWorldSubsystem::ResolveKeyToLocalId(const FFlecsReplicationKey& Key, FFlecsId& OutId) const
{
	const TSolidNotNull<const UFlecsWorld*> World = GetFlecsWorldChecked();
	const FFlecsComponentReplicationRegistry& Registry = FFlecsComponentReplicationRegistry::Get(World);
	const FFlecsComponentReplicationDescriptor* Storage = Registry.Find(Key.StorageSchema);
	
	if (!Storage)
	{
		return false;
	}
	
	if (Key.Kind == EFlecsReplicationKeyKind::Component)
	{
		OutId = Storage->LocalFlecsId;
		return true;
	}
	
	const FFlecsComponentReplicationDescriptor* Relationship = Registry.Find(Key.RelationshipSchema);
	
	if (!Relationship)
	{
		return false;
	}

	FFlecsId Target;
	switch (Key.TargetKind)
	{
	case EFlecsReplicationPairTargetKind::Schema:
		if (const FFlecsComponentReplicationDescriptor* TargetDescriptor = Registry.Find(Key.TargetSchema))
		{
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
	if (Layout.Keys.Num() > MAX_uint16)
	{
		OutError = TEXT("Received replication layout exceeds the key limit");
		return false;
	}
	
	for (const FFlecsReplicationKey& Key : Layout.Keys)
	{
		auto ValidateSchema = [&Registry, &OutError](const FFlecsReplicationSchemaId Schema, const TCHAR* Role)
		{
			const FFlecsComponentReplicationDescriptor* Descriptor = Registry.Find(Schema);
			
			if (!Descriptor)
			{
				OutError = FString::Printf(TEXT("%s schema %s is not found in the local schema set"),
					Role, *Schema.ToString());
				return false;
			}
			
			return true;
		};
		
		if (!ValidateSchema(Key.StorageSchema, TEXT("Storage")))
		{
			return false;
		}
		if (const FFlecsComponentReplicationDescriptor* Storage = Registry.Find(Key.StorageSchema);
			!Storage || Storage->CodecFingerprint != Key.CodecFingerprint)
		{
			OutError = FString::Printf(TEXT("Storage schema %s codec fingerprint does not match"),
				*Key.StorageSchema.ToString());
			return false;
		}
		
		if (Key.Kind == EFlecsReplicationKeyKind::Pair
			&& !ValidateSchema(Key.RelationshipSchema, TEXT("Relationship")))
		{
			return false;
		}
		
		if (Key.TargetKind == EFlecsReplicationPairTargetKind::Schema
			&& !ValidateSchema(Key.TargetSchema, TEXT("Target")))
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

double UFlecsNetworkWorldSubsystem::GetReplicationTimeSeconds() const
{
#if WITH_AUTOMATION_TESTS || WITH_EDITOR
	if (TestingTimeSeconds.IsSet())
	{
		return TestingTimeSeconds.GetValue();
	}
#endif
	return FPlatformTime::Seconds();
}
