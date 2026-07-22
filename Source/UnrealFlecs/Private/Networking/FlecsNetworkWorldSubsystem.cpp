// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsNetworkWorldSubsystem.h"

#include "Engine/World.h"
#include "Networking/FlecsNetDirtyTag.h"

#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

#include "Networking/FlecsNetRoleType.h"
#include "Networking/FlecsNetworkingModuleSettings.h"
#include "Networking/FlecsNetworkSubsystemSingleton.h"
#include "Networking/FlecsReplicatedEntityComponent.h"

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
	
	FFlecsComponentReplicationRegistry::Get(InWorld).OnDescriptorRegistered()
		.AddUObject(this, &UFlecsNetworkWorldSubsystem::RegisterIndividualComponentDirtyObserver);
}

void UFlecsNetworkWorldSubsystem::Deinitialize()
{
	FFlecsComponentReplicationRegistry::RemoveWorld(GetFlecsWorld());
	
	Super::Deinitialize();
}

void UFlecsNetworkWorldSubsystem::RegisterComponentDirtyObservers()
{
	const FFlecsComponentReplicationRegistry& Registry = FFlecsComponentReplicationRegistry::Get(GetFlecsWorld());
	const TMap<FFlecsId, FFlecsComponentReplicationDescriptor>& Descriptors = Registry.GetDescriptors();
	
	for (const TTuple<FFlecsId, FFlecsComponentReplicationDescriptor>& Pair : Descriptors)
	{
		RegisterIndividualComponentDirtyObserver(Pair.Value);
	}
}

void UFlecsNetworkWorldSubsystem::RegisterIndividualComponentDirtyObserver(
	const FFlecsComponentReplicationDescriptor& InDescriptor)
{
	if UNLIKELY_IF(!InDescriptor.IsValid())
	{
		return;
	}
	
	if (InDescriptor.bIsTag)
	{
		return;
	}
	
	auto CreateObserver = [this](const FFlecsId InComponentId) -> FFlecsObserverHandle
	{
		return GetFlecsWorld()->CreateObserver<FFlecsReplicatedEntityComponent>()
			.With(InComponentId)
			.With<FFlecsReplicatedEntityComponent>().Filter()
			.Event(flecs::OnSet)
			.Event(flecs::OnAdd)
			.Event(flecs::OnRemove)
			.each([this](flecs::iter& Iter, size_t Index, const FFlecsReplicatedEntityComponent& InReplicatedEntityComponent)
			{
				const FFlecsEntityHandle EntityHandle = Iter.entity(Index);
				solid_check(EntityHandle.IsValid());
				
				EntityHandle.Add<FFlecsNetDirtyTag>();
			});
	};
	
	const FFlecsObserverHandle PrimaryObserverHandle = CreateObserver(InDescriptor.LocalFlecsId);
	const FFlecsObserverHandle PairObserverHandle = CreateObserver(
		FFlecsId::MakePair(InDescriptor.LocalFlecsId, flecs::Wildcard));
	
	ComponentDirtyObservers.Add(PrimaryObserverHandle);
	ComponentDirtyObservers.Add(PairObserverHandle);
}

FFlecsNetworkId UFlecsNetworkWorldSubsystem::BeginReplicatingEntity(const FFlecsEntityHandle& EntityHandle)
{
	if UNLIKELY_IF(!HasAuthority())
	{
		UE_LOG(LogFlecsWorld, Error, 
			TEXT("Cannot begin replicating entity %s without authority"), *EntityHandle.ToString());
		return FFlecsNetworkId();
	}
	
	EntityHandle.
}

void UFlecsNetworkWorldSubsystem::StopReplicatingEntity(const FFlecsEntityHandle& EntityHandle)
{
	
}

void UFlecsNetworkWorldSubsystem::StopReplicatingEntity(const FFlecsNetworkId NetworkId)
{
	
}

bool UFlecsNetworkWorldSubsystem::HasAuthority() const
{
	return GetWorld()->GetNetMode() != NM_Client;
}

bool UFlecsNetworkWorldSubsystem::IsStandalone() const
{
	return GetWorld()->GetNetMode() == NM_Standalone;
}
