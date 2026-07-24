// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsNetworkWorldSubsystem.h"

#include "Engine/World.h"
#include "Networking/FlecsDirtyObserverTag.h"
#include "Networking/FlecsNetDirtyTag.h"

#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

#include "Networking/FlecsNetRoleType.h"
#include "Networking/FlecsNetworkIDGeneratorInterface.h"
#include "Networking/FlecsNetworkingModuleSettings.h"
#include "Networking/FlecsNetworkSubsystemSingleton.h"
#include "Networking/FlecsReplicatedEntityComponent.h"
#include "Networking/FlecsReplicationBridgeBase.h"

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
	
	CreateReplicationBridge();
	
	
#endif // WITH_SERVER_CODE
	
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
	ReplicationBridge->InitializeBridge();
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
	const TArray<TPair<FFlecsEntityHandle, FFlecsEntityReplicationSnapshot>>* DeferredSnapshotsPtr
		= DeferredEntityLayouts.Find(InLayout.LayoutId);

	if (!DeferredSnapshotsPtr)
	{
		return;
	}
	
	for (const TPair<FFlecsEntityHandle, FFlecsEntityReplicationSnapshot>& Pair : *DeferredSnapshotsPtr)
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
		
	const FFlecsEntityReplicationSnapshot& ExistingSnapshot = GetReplicationSnapshots().FindOrAdd(InNetworkId);
		
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
		AddDeferredEntityLayout(EntityHandle, InSnapshot.LayoutId, InSnapshot);
		return;
	}
		
	ApplySnapshotToEntity(EntityHandle, InSnapshot);
}

void UFlecsNetworkWorldSubsystem::ApplySnapshotToEntity(const FFlecsEntityHandle& InEntityHandle,
	const FFlecsEntityReplicationSnapshot& InSnapshot)
{
	//FFlecsScopedDeferWindow DeferWindow(InEntityHandle.GetFlecsWorldChecked());
	
	for (const FFlecsReplicationKey& Key : GetLayoutRegistry().Find(InSnapshot.LayoutId)->Keys)
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
		if UNLIKELY_IF(!LayoutRegistry.Find(InSnapshot.LayoutId))
		{
			UE_LOG(LogFlecsWorld, Error,
				TEXT("Cannot apply snapshot to entity %s because layout ID '%s' is not registered"),
				*InEntityHandle.ToString(), *InSnapshot.LayoutId.ToString());
			continue;
		}
		
		const FFlecsReplicationKey& Key = LayoutRegistry.Find(InSnapshot.LayoutId)->Keys[Value.KeyIndex];
		
		const FFlecsId ComponentId = FFlecsReplicationKey::ResolveToId(GetFlecsWorldChecked(), Key);
		
		if UNLIKELY_IF(!ComponentId.IsValid())
		{
			UE_LOG(LogFlecsWorld, Error,
				TEXT("Cannot apply snapshot to entity %s because component ID for key '%s' is not valid"),
				*InEntityHandle.ToString(), *Key.CanonicalString());
			continue;
		}
		
		InEntityHandle.Set(ComponentId, Value.Bytes.GetData());
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
