// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"

#include "Engine/World.h"
#include "Networking/FlecsDirtyObserverTag.h"
#include "Networking/FlecsNetDirtyTag.h"

#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

#include "Networking/FlecsNetRoleType.h"
#include "Networking/FlecsNetworkIDGeneratorInterface.h"
#include "Networking/FlecsNetworkingModuleSettings.h"
#include "Networking/Subsystem/FlecsNetworkSubsystemSingleton.h"
#include "Networking/FlecsReplicatedEntityComponent.h"
#include "Networking/Bridge/FlecsReplicationBridgeBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetworkWorldSubsystem)

UFlecsNetworkWorldSubsystem::UFlecsNetworkWorldSubsystem()
{
}

void UFlecsNetworkWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	
}

void UFlecsNetworkWorldSubsystem::OnFlecsWorldInitialized(const TSolidNotNull<UFlecsWorld*> InWorld)
{
	Super::OnFlecsWorldInitialized(InWorld);
	
	InWorld->Set<FFlecsNetworkSubsystemSingleton>(FFlecsNetworkSubsystemSingleton{ this });
	
#if WITH_SERVER_CODE
	
	CreateNetworkIdGenerator();
	
	FFlecsComponentReplicationRegistry::Get(InWorld).OnDescriptorRegistered()
		.AddUObject(this, &UFlecsNetworkWorldSubsystem::RegisterIndividualComponentDirtyObserver);
	
#endif // WITH_SERVER_CODE

	CreateReplicationBridge();
}

void UFlecsNetworkWorldSubsystem::Deinitialize()
{
	if (ReplicationBridge)
	{
		ReplicationBridge->DeinitializeBridge();
		ReplicationBridge = nullptr;
	}

	FFlecsComponentReplicationRegistry::RemoveWorld(GetFlecsWorld());
	
	Super::Deinitialize();
}

void UFlecsNetworkWorldSubsystem::RegisterComponentDirtyObservers()
{
	if (!HasAuthority())
	{
		return;
	}
	
	const FFlecsComponentReplicationRegistry& Registry = FFlecsComponentReplicationRegistry::Get(GetFlecsWorld());
	const TMap<FFlecsId, FFlecsComponentReplicationDescriptor>& Descriptors = Registry.GetDescriptors();
	
	for (const TTuple<FFlecsId, FFlecsComponentReplicationDescriptor>& Pair : Descriptors)
	{
		RegisterIndividualComponentDirtyObserver(Pair.Value);
	}
}

void UFlecsNetworkWorldSubsystem::RegisterIndividualComponentDirtyObserver(const FFlecsComponentReplicationDescriptor& InDescriptor)
{
	// This should have been checked previously
	if UNLIKELY_IF(!HasAuthority())
	{
		UE_LOG(LogFlecsWorld, Error, 
			TEXT("Cannot register component dirty observer without authority"));
		
		return;
	}
	
	if UNLIKELY_IF(!InDescriptor.IsValid())
	{
		return;
	}
	
	if (InDescriptor.bIsTag)
	{
		return;
	}
	
	auto CreateObserver = [this](
		const FFlecsId InFirstId,
		const FFlecsId InSecondId = FFlecsId()) -> FFlecsObserverHandle
	{
		TFlecsObserverBuilder<> ObserverBuilder = GetFlecsWorld()->CreateObserver();

		if (InSecondId.IsValid())
		{
			ObserverBuilder.WithPair(InFirstId, InSecondId);
		}
		else
		{
			ObserverBuilder.With(InFirstId);
		}

		const FFlecsObserverHandle DirtyObserverHandle = ObserverBuilder
			.With<FFlecsReplicatedEntityComponent>().Filter()
			.Event(flecs::OnSet)
			.Event(flecs::OnAdd)
			.Event(flecs::OnRemove)
			.each([this](flecs::iter& Iter, size_t Index)
			{
				const FFlecsEntityHandle EntityHandle = Iter.entity(Index);
				solid_check(EntityHandle.IsValid());
				
				EntityHandle.Add<FFlecsNetDirtyTag>();
			});
		
		DirtyObserverHandle.Add<FFlecsDirtyObserverTag>();
		
		return DirtyObserverHandle;
	};
	
	const FFlecsObserverHandle PrimaryObserverHandle = CreateObserver(InDescriptor.LocalFlecsId);
	const FFlecsObserverHandle PairFirstObserverHandle =
		CreateObserver(InDescriptor.LocalFlecsId, flecs::Wildcard);
	/*const FFlecsObserverHandle PairSecondObserverHandle =
		CreateObserver(flecs::Wildcard, InDescriptor.LocalFlecsId);*/
	
	ComponentDirtyObservers.Add(PrimaryObserverHandle);
	ComponentDirtyObservers.Add(PairFirstObserverHandle);
	/*ComponentDirtyObservers.Add(PairSecondObserverHandle);*/
}

FFlecsNetworkId UFlecsNetworkWorldSubsystem::BeginReplicatingEntity(const FFlecsEntityHandle& InEntityHandle)
{
	if UNLIKELY_IF(!ensureAlwaysMsgf(InEntityHandle.IsValid(), TEXT("Entity handle is not valid")))
	{
		return FFlecsNetworkId();
	}
	
	if UNLIKELY_IF(!HasAuthority())
	{
		UE_LOG(LogFlecsWorld, Error, 
			TEXT("Cannot begin replicating entity %s without authority"), *InEntityHandle.ToString());
		return FFlecsNetworkId();
	}
	
	const FFlecsNetworkId* ExistingNetworkId = InEntityHandle.TryGet<FFlecsNetworkId>();
	
	if UNLIKELY_IF(ExistingNetworkId && ExistingNetworkId->IsValid())
	{
		UE_LOG(LogFlecsWorld, Error, 
			TEXT("Entity %s is already replicating with network ID '%s'"), 
			*InEntityHandle.ToString(), *ExistingNetworkId->ToString());
		
		return *ExistingNetworkId;
	}
	
	const FFlecsNetworkId NetworkId = GetNetworkIdGenerator()->GenerateNetworkId();
	
	if UNLIKELY_IF(!ensureMsgf(NetworkId.IsValid(), TEXT("Generated network ID is not valid")))
	{
		return FFlecsNetworkId();
	}
	
	InEntityHandle.Set<FFlecsNetworkId>(NetworkId);
	InEntityHandle.Add<EFlecsNetRoleType>(EFlecsNetRoleType::Authority);
	InEntityHandle.Add<FFlecsNetDirtyTag>();
	
	NetworkIdToEntityMap.Add(NetworkId, InEntityHandle);

	return NetworkId;
}

void UFlecsNetworkWorldSubsystem::StopReplicatingEntity(const FFlecsEntityHandle& InEntityHandle)
{
	if UNLIKELY_IF(!HasAuthority())
	{
		UE_LOG(LogFlecsWorld, Error,
			TEXT("Cannot stop replicating entity %s without authority"),
			*InEntityHandle.ToString());
		return;
	}

	const FFlecsNetworkId* NetworkId = InEntityHandle.TryGet<FFlecsNetworkId>();
	if (!NetworkId || !NetworkId->IsValid())
	{
		return;
	}

	if (!NetworkIdToEntityMap.Contains(*NetworkId))
	{
		return;
	}

	if (ReplicationBridge)
	{
		ReplicationBridge->StopReplicatingEntity(InEntityHandle);
	}

	NetworkIdToEntityMap.Remove(*NetworkId);
	ReplicationSnapshots.Remove(*NetworkId);

	if (NetworkIdGenerator)
	{
		GetNetworkIdGenerator()->ReleaseNetworkId(*NetworkId);
	}

	InEntityHandle.Remove<FFlecsNetworkId>();
	InEntityHandle.Remove<EFlecsNetRoleType>();
}

void UFlecsNetworkWorldSubsystem::CreateNetworkIdGenerator()
{
	if (!HasAuthority())
	{
		return;
	}
	
	const TSolidNotNull<const UFlecsNetworkingModuleSettings*> Settings = GetNetworkingSettings();
	
	NetworkIdGenerator = NewObject<UObject>(this, Settings->NetworkIdGeneratorClass);
	solid_checkf(IsValid(NetworkIdGenerator), TEXT("Network ID generator is not valid"));
}

void UFlecsNetworkWorldSubsystem::CreateReplicationBridge()
{
	if (!HasAuthority())
	{
		return;
	}

	const TSolidNotNull<const UFlecsNetworkingModuleSettings*> Settings = GetNetworkingSettings();
	
	if UNLIKELY_IF(!ensureMsgf(Settings->ReplicationBridgeClass, TEXT("Replication bridge class is not set in settings")))
	{
		return;
	}
	
	ReplicationBridge = NewObject<UFlecsReplicationBridgeBase>(this, Settings->ReplicationBridgeClass);
	solid_checkf(IsValid(ReplicationBridge), TEXT("Replication bridge is not valid"));
	
	ReplicationBridge->SetNetworkWorldSubsystem(this);
	ReplicationBridge->InitializeBridge();
}

void UFlecsNetworkWorldSubsystem::BindReplicationBridge(const TSolidNotNull<UFlecsReplicationBridgeBase*> InReplicationBridge)
{
	solid_check(!HasAuthority());

	if (ReplicationBridge == InReplicationBridge)
	{
		return;
	}

	if (ReplicationBridge)
	{
		ReplicationBridge->DeinitializeBridge();
	}

	ReplicationBridge = InReplicationBridge;
	ReplicationBridge->SetNetworkWorldSubsystem(this);
	ReplicationBridge->InitializeBridge();
}

void UFlecsNetworkWorldSubsystem::UnbindReplicationBridge(const UFlecsReplicationBridgeBase* InReplicationBridge)
{
	check(!HasAuthority());

	if (ReplicationBridge != InReplicationBridge)
	{
		return;
	}

	ReplicationBridge->DeinitializeBridge();
	ReplicationBridge = nullptr;
}

TSolidNotNull<IFlecsNetworkIDGeneratorInterface*> UFlecsNetworkWorldSubsystem::GetNetworkIdGenerator() const
{
	return CastChecked<IFlecsNetworkIDGeneratorInterface>(NetworkIdGenerator);
}

TSolidNotNull<UFlecsReplicationBridgeBase*> UFlecsNetworkWorldSubsystem::GetReplicationBridge() const
{
	solid_cassumef(ReplicationBridge, TEXT("Replication bridge is not valid"));
	return ReplicationBridge;
}

bool UFlecsNetworkWorldSubsystem::HasReplicationBridge() const
{
	return IsValid(ReplicationBridge);
}

#if WITH_AUTOMATION_TESTS

void UFlecsNetworkWorldSubsystem::SetReplicationBridgeForTesting(UFlecsReplicationBridgeBase* InReplicationBridge)
{
	if (ReplicationBridge == InReplicationBridge)
	{
		return;
	}

	if (ReplicationBridge)
	{
		ReplicationBridge->DeinitializeBridge();
	}

	ReplicationBridge = InReplicationBridge;

	if (ReplicationBridge)
	{
		ReplicationBridge->SetNetworkWorldSubsystem(this);
		ReplicationBridge->InitializeBridge();
	}
}

#endif // WITH_AUTOMATION_TESTS

bool UFlecsNetworkWorldSubsystem::HasAuthority() const
{
	return GetWorld()->GetNetMode() != NM_Client;
}

bool UFlecsNetworkWorldSubsystem::IsStandalone() const
{
	return GetWorld()->GetNetMode() == NM_Standalone;
}

void UFlecsNetworkWorldSubsystem::OnEntityLayoutReceived(const FFlecsReplicationLayoutDefinition& InLayout)
{
	TArray<TPair<FFlecsEntityHandle, FFlecsEntityReplicationSnapshot>>* DeferredSnapshotsPtr
		= DeferredEntityLayouts.Find(InLayout.LayoutId);

	if (!DeferredSnapshotsPtr)
	{
		return;
	}

	TArray<TPair<FFlecsEntityHandle, FFlecsEntityReplicationSnapshot>> DeferredSnapshots =
		MoveTemp(*DeferredSnapshotsPtr);
	DeferredEntityLayouts.Remove(InLayout.LayoutId);
	
	for (const TPair<FFlecsEntityHandle, FFlecsEntityReplicationSnapshot>& Pair : DeferredSnapshots)
	{
		const FFlecsEntityHandle& EntityHandle = Pair.Key;
		const FFlecsEntityReplicationSnapshot& Snapshot = Pair.Value;

		if UNLIKELY_IF(!ensureAlwaysMsgf(EntityHandle.IsValid(), TEXT("Deferred entity handle is not valid")))
		{
			continue;
		}

		ApplySnapshotToEntity(EntityHandle, Snapshot);
	}
}

void UFlecsNetworkWorldSubsystem::ReceiveNetworkEntitySnapshot(const FFlecsNetworkId& InNetworkId,
                                                               const FFlecsEntityReplicationSnapshot& InSnapshot)
{
	if UNLIKELY_IF(!InNetworkId.IsValid() || !InSnapshot.LayoutId.IsValid())
	{
		UE_LOG(LogFlecsWorld, Error, TEXT("Received an invalid Flecs entity snapshot"));
		return;
	}

	const TOptional<FFlecsEntityHandle> EntityHandlePtr = GetEntityFromNetworkId(InNetworkId);
	
	FFlecsEntityHandle EntityHandle;
	if (EntityHandlePtr.IsSet())
	{
		EntityHandle = EntityHandlePtr.GetValue();
		solid_checkf(EntityHandle.IsValid(), TEXT("Entity handle for network ID '%s' is not valid"), *InNetworkId.ToString());
	}
	else
	{
		EntityHandle = GetFlecsWorldChecked()->CreateEntity()
			.Set<FFlecsNetworkId>(InNetworkId)
			.Add<EFlecsNetRoleType>(EFlecsNetRoleType::SimulatedProxy);
	
		NetworkIdToEntityMap.Add(InNetworkId, EntityHandle);
	}
		
	FFlecsEntityReplicationSnapshot& ExistingSnapshot = GetReplicationSnapshots().FindOrAdd(InNetworkId);
		
	if (ExistingSnapshot.StateRevision >= InSnapshot.StateRevision)
	{
		UE_LOG(LogFlecsWorld, Warning,
			TEXT("Received replication snapshot for network ID '%s' with state revision %d, but existing snapshot has state revision %d"),
			*InNetworkId.ToString(), InSnapshot.StateRevision, ExistingSnapshot.StateRevision);
		return;
	}
		
	const FFlecsReplicationLayoutDefinition* LayoutDefinition = GetLayoutRegistry().Find(InSnapshot.LayoutId);
	if (!LayoutDefinition)
	{
		ExistingSnapshot = InSnapshot;
		AddDeferredEntityLayout(EntityHandle, InSnapshot.LayoutId, InSnapshot);
		return;
	}
		
	ApplySnapshotToEntity(EntityHandle, InSnapshot);
	ExistingSnapshot = InSnapshot;
}

void UFlecsNetworkWorldSubsystem::RemoveReceivedNetworkEntity(const FFlecsNetworkId& InNetworkId)
{
	if (HasAuthority())
	{
		return;
	}

	if (!InNetworkId.IsValid())
	{
		return;
	}

	if (FFlecsEntityHandle* EntityHandle = NetworkIdToEntityMap.Find(InNetworkId))
	{
		if (EntityHandle->IsValid())
		{
			EntityHandle->Destroy();
		}

		NetworkIdToEntityMap.Remove(InNetworkId);
	}

	ReplicationSnapshots.Remove(InNetworkId);

	for (auto It = DeferredEntityLayouts.CreateIterator(); It; ++It)
	{
		It.Value().RemoveAll(
			[&InNetworkId](const TPair<FFlecsEntityHandle, FFlecsEntityReplicationSnapshot>& Pair)
			{
				const FFlecsNetworkId* PairNetworkId = Pair.Key.TryGet<FFlecsNetworkId>();
				return PairNetworkId && *PairNetworkId == InNetworkId;
			});

		if (It.Value().IsEmpty())
		{
			It.RemoveCurrent();
		}
	}
}

void UFlecsNetworkWorldSubsystem::ApplySnapshotToEntity(const FFlecsEntityHandle& InEntityHandle,
	const FFlecsEntityReplicationSnapshot& InSnapshot)
{
	//FFlecsScopedDeferWindow DeferWindow(InEntityHandle.GetFlecsWorldChecked());

	const FFlecsReplicationLayoutDefinition* LayoutDefinition = GetLayoutRegistry().Find(InSnapshot.LayoutId);
	if UNLIKELY_IF(!LayoutDefinition)
	{
		UE_LOG(LogFlecsWorld, Error,
			TEXT("Cannot apply snapshot to entity %s because layout ID '%s' is not registered"),
			*InEntityHandle.ToString(), *InSnapshot.LayoutId.ToString());
		return;
	}

	for (const FFlecsReplicationKey& Key : LayoutDefinition->Keys)
	{
		const FFlecsId ComponentId = FFlecsReplicationKey::ResolveToId(GetFlecsWorldChecked(), Key);
		
		if UNLIKELY_IF(!ComponentId.IsValid())
		{
			UE_LOG(LogFlecsWorld, Error,
				TEXT("Cannot apply snapshot to entity %s because component ID for key '%s' is not valid"),
				*InEntityHandle.ToString(), *Key.CanonicalString());
			continue;
		}
		
		InEntityHandle.Remove(ComponentId);
	}
	
	for (const FFlecsReplicatedValue& Value : InSnapshot.Values)
	{
		if UNLIKELY_IF(!LayoutDefinition->Keys.IsValidIndex(Value.KeyIndex))
		{
			UE_LOG(LogFlecsWorld, Error,
				TEXT("Cannot apply snapshot to entity %s because value key index %d is outside layout '%s'"),
				*InEntityHandle.ToString(), Value.KeyIndex, *InSnapshot.LayoutId.ToString());
			continue;
		}
		
		const FFlecsReplicationKey& Key = LayoutDefinition->Keys[Value.KeyIndex];
		
		const FFlecsId ComponentId = FFlecsReplicationKey::ResolveToId(GetFlecsWorldChecked(), Key);
		
		if UNLIKELY_IF(!ComponentId.IsValid())
		{
			UE_LOG(LogFlecsWorld, Error,
				TEXT("Cannot apply snapshot to entity %s because component ID for key '%s' is not valid"),
				*InEntityHandle.ToString(), *Key.CanonicalString());
			continue;
		}
		
		const FFlecsComponentReplicationDescriptor* Descriptor =
			FFlecsComponentReplicationRegistry::Get(GetFlecsWorldChecked()).Find(ComponentId);
		if UNLIKELY_IF(!Descriptor || !Descriptor->GetDeserializeFunction()
			|| !Descriptor->GetConstructFunction() || !Descriptor->GetDestroyFunction())
		{
			UE_LOG(LogFlecsWorld, Error,
				TEXT("Cannot deserialize snapshot value for entity %s and component key '%s'"),
				*InEntityHandle.ToString(), *Key.CanonicalString());
			continue;
		}

		void* ComponentData = FMemory::Malloc(Descriptor->GetSize(), Descriptor->GetAlignment());
		if UNLIKELY_IF(!ComponentData)
		{
			UE_LOG(LogFlecsWorld, Error,
				TEXT("Cannot allocate snapshot value for entity %s and component key '%s'"),
				*InEntityHandle.ToString(), *Key.CanonicalString());
			continue;
		}

		Descriptor->GetConstructFunction()(ComponentData);

		FMemoryReader Reader(Value.Bytes, true);
		const bool bDeserialized = Descriptor->GetDeserializeFunction()(Reader, ComponentData);
		if UNLIKELY_IF(!bDeserialized || Reader.IsError())
		{
			UE_LOG(LogFlecsWorld, Error,
				TEXT("Cannot deserialize snapshot value for entity %s and component key '%s'"),
				*InEntityHandle.ToString(), *Key.CanonicalString());
			Descriptor->GetDestroyFunction()(ComponentData);
			FMemory::Free(ComponentData);
			continue;
		}

		InEntityHandle.Set(ComponentId, Descriptor->GetSize(), ComponentData);
		Descriptor->GetDestroyFunction()(ComponentData);
		FMemory::Free(ComponentData);
	}
}

void UFlecsNetworkWorldSubsystem::AddDeferredEntityLayout(const FFlecsEntityHandle& InEntityHandle,
	const FFlecsReplicationLayoutId& InLayout, const FFlecsEntityReplicationSnapshot& InSnapshot)
{
	if UNLIKELY_IF(!ensureAlwaysMsgf(InEntityHandle.IsValid(), TEXT("Entity handle is not valid")))
	{
		return;
	}
	
	TArray<TPair<FFlecsEntityHandle, FFlecsEntityReplicationSnapshot>>& DeferredSnapshots = DeferredEntityLayouts.FindOrAdd(InLayout);
	DeferredSnapshots.Add(TPair<FFlecsEntityHandle, FFlecsEntityReplicationSnapshot>(InEntityHandle, InSnapshot));
}

TSolidNotNull<const UFlecsNetworkingModuleSettings*> UFlecsNetworkWorldSubsystem::GetNetworkingSettings()
{
	return GetDefault<UFlecsNetworkingModuleSettings>();
}
