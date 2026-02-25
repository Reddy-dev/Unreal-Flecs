// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Entities/FlecsSubEntityRecordNameObserver.h"

#include "Components/FlecsSubEntityRecordNameComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsSubEntityRecordNameObserver)

UFlecsSubEntityRecordNameObserver::UFlecsSubEntityRecordNameObserver(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlecsSubEntityRecordNameObserver::BuildObserver(const TSolidNotNull<const UFlecsWorld*> InWorld,
	TFlecsObserverBuilder<>& InOutBuilder) const
{
	InOutBuilder
		.Event(flecs::OnSet)
		.With<const FFlecsSubEntityRecordNameComponent>() // 0
		.With<flecs::Parent>().Filter() // 1
		.WithoutPair<flecs::Identifier>(flecs::Name) // 2
		.YieldExisting();
}

void UFlecsSubEntityRecordNameObserver::EachIterator(const TSolidNotNull<const UFlecsWorld*> InWorld,
	flecs::iter& InIterator, const FFlecsId InIndex)
{
	const FFlecsEntityHandle EntityHandle = InIterator.entity(InIndex);
	const auto& [SubEntityName] = InIterator.field_at<const FFlecsSubEntityRecordNameComponent>(0, InIndex);
	
	EntityHandle.SetName(SubEntityName);
}
