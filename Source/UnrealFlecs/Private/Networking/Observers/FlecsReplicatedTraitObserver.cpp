// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Observers/FlecsReplicatedTraitObserver.h"

#include "Networking/FlecsReplicatedTrait.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicatedTraitObserver)

UFlecsReplicatedTraitObserver::UFlecsReplicatedTraitObserver()
{
}

void UFlecsReplicatedTraitObserver::BuildObserver(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	TFlecsObserverBuilder<>& InOutBuilder) const
{
	// @TODO: Support Removal?
	InOutBuilder
		.Event(flecs::OnAdd)
		.With<FFlecsReplicatedTrait>()
		.WithPair<flecs::Identifier>(flecs::Symbol);
}

void UFlecsReplicatedTraitObserver::EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	flecs::iter& InIterator, const FFlecsId InIndex)
{
	
}
