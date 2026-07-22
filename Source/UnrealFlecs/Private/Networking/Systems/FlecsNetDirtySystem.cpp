// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Systems/FlecsNetDirtySystem.h"

#include "Networking/FlecsNetDirtyTag.h"
#include "Networking/FlecsNetworkSubsystemSingleton.h"
#include "Networking/FlecsNetworkWorldSubsystem.h"
#include "Networking/FlecsReplicatedEntityComponent.h"
#include "Networking/Layout/FlecsReplicationSnapshot.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetDirtySystem)

UFlecsNetDirtySystem::UFlecsNetDirtySystem()
{
}

void UFlecsNetDirtySystem::BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld,
                                       TFlecsSystemBuilder<>& InBuilder) const
{
	InBuilder
		.With<FFlecsNetDirtyTag>().ReadWrite() // 0
		.With<FFlecsReplicatedEntityComponent&>() // 1
		.With<const FFlecsNetworkId>() // 2
		.With<FFlecsNetworkSubsystemSingleton&>(); // 3
}

void UFlecsNetDirtySystem::EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	flecs::iter& InIterator, const FFlecsId InIndex)
{
	auto ReplicatedComponentsField = InIterator.field<FFlecsReplicatedEntityComponent>(1);
	auto NetworkIdsField = InIterator.field<const FFlecsNetworkId>(2);
	auto NetworkSubsystemField = InIterator.field<FFlecsNetworkSubsystemSingleton>(3);
	
	const TSolidNotNull<UFlecsNetworkWorldSubsystem*> NetworkSubsystem
		= NetworkSubsystemField->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
	
	for (const FFlecsId EntityIndex : InIterator)
	{
		FFlecsReplicatedEntityComponent& ReplicatedComponent = ReplicatedComponentsField[EntityIndex];
		const FFlecsNetworkId& NetworkId = NetworkIdsField[EntityIndex];
		
		const FFlecsEntityHandle EntityHandle = InIterator.entity(EntityIndex);
		
		TValueOrError<const FFlecsReplicationLayoutDefinition*, FString> LayoutResult = 
			NetworkSubsystem->GetLayoutRegistry().BuildForEntity(InWorld, EntityHandle);
		
		// @TODO: Remove this in shipping?
		if UNLIKELY_IF(LayoutResult.HasError())
		{
			UE_LOG(LogFlecsCore, Error, TEXT("Failed to build replication layout for entity %s: %s"),
				*EntityHandle.ToString(), *LayoutResult.GetError());
			EntityHandle.Remove<FFlecsNetDirtyTag>();
			
			continue;
		}
		
		// @TODO: DontFragment
		
		const FFlecsReplicationLayoutId NewLayoutId = LayoutResult.GetValue()->LayoutId;
		
		ReplicatedComponent.LayoutId = NewLayoutId;
		
		FFlecsEntityReplicationSnapshot& Snapshot = NetworkSubsystem->GetReplicationSnapshots().FindOrAdd(NetworkId);
		Snapshot.LayoutId = NewLayoutId;

		EntityHandle.Remove<FFlecsNetDirtyTag>();
	}
}
