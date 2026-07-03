// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Observers/FlecsReplicatedComponentObservers.h"

#include "Networking/FlecsNetworkSubsystemSingleton.h"
#include "Networking/FlecsReplicatedComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicatedComponentObservers)

UFlecsReplicatedComponentObservers::UFlecsReplicatedComponentObservers()
{
	
}

void UFlecsReplicatedComponentObservers::BuildObserver(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
                                                       TFlecsObserverBuilder<>& InOutBuilder) const
{
	InOutBuilder
		.Event(flecs::OnAdd)
		.With<FFlecsReplicatedComponent>() // 0
		.With<FFlecsNetworkSubsystemSingleton>(); // 1
}

void UFlecsReplicatedComponentObservers::EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	flecs::iter& InIterator, const FFlecsId InIndex)
{
	
	for (const FFlecsId EntityIndex : InIterator)
	{
		
	}
}
