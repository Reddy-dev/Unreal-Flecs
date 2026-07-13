// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Observers/FlecsReplicatedComponentObservers.h"

#include "Networking/FlecsNetworkSubsystemSingleton.h"
#include "Networking/FlecsNetworkWorldSubsystem.h"
#include "Networking/FlecsReplicatedEntityComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicatedComponentObservers)

UFlecsReplicatedComponentObservers::UFlecsReplicatedComponentObservers()
{
	
}

void UFlecsReplicatedComponentObservers::BuildObserver(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
                                                       TFlecsObserverBuilder<>& InOutBuilder) const
{
	InOutBuilder
		.Event(flecs::OnAdd)
		.Event(flecs::OnRemove)
		.With<FFlecsReplicatedEntityComponent>() // 0
		.With<FFlecsNetworkSubsystemSingleton>() // 1
		.Without<FFlecsReplicatedTrait>()
		.YieldExisting();
}

void UFlecsReplicatedComponentObservers::EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	flecs::iter& InIterator, const FFlecsId InIndex)
{
	//const auto ReplicatedComponentField = InIterator.field<FFlecsReplicatedComponent>(0);
	const auto NetworkSubsystem = InIterator.field<FFlecsNetworkSubsystemSingleton>(1);
	
	const TSolidNotNull<UFlecsNetworkWorldSubsystem*> NetworkSubsystemPtr = NetworkSubsystem->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
	
	for (const FFlecsId EntityIndex : InIterator)
	{
		const FFlecsEntityHandle EntityHandle = InIterator.entity(EntityIndex);
		
		if (InIterator.event() == flecs::OnRemove)
		{
			NetworkSubsystemPtr->StopReplicatingEntity(EntityHandle);
			continue;
		}
		
		NetworkSubsystemPtr->BeginReplicatingEntity(EntityHandle);
	}
}

EFlecsObjectRegistrationNetworkFlags UFlecsReplicatedComponentObservers::GetObjectRegistrationNetworkFlags() const
{
	return EFlecsObjectRegistrationNetworkFlags::Server;
}
