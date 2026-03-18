// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Pipelines/FlecsTickFunctionObservers.h"

#include "Pipelines/TickFunctions/FlecsTickFunctionComponent.h"
#include "Pipelines/TickFunctions/FlecsTickTypeRelationship.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsTickFunctionObservers)

void UFlecsAddTickFunctionObserver::BuildObserver(const TSolidNotNull<UFlecsWorld*> InWorld,
	TFlecsObserverBuilder<>& InOutBuilder) const
{
	InOutBuilder
		.Event(flecs::OnAdd)
		.With<FFlecsTickFunctionComponent&>() // 0
		.WithPair<FFlecsTickTypeRelationship>("$TickTypeTag") // 1
		.YieldExisting();
}

void UFlecsAddTickFunctionObserver::EachIterator(const TSolidNotNull<UFlecsWorld*> InWorld,
	flecs::iter& InIterator, const FFlecsId InIndex)
{
	const FFlecsTickFunctionComponent& InTickFunctionComponent = InIterator.field_at<FFlecsTickFunctionComponent>(0, InIndex);
	
	solid_checkf(InTickFunctionComponent.TickFunction.IsValid(),
				TEXT("FFlecsTickFunctionComponent has invalid TickFunction"));

#if !NO_LOGGING
	const FFlecsEntityHandle EntityHandle = InIterator.entity(InIndex);
#endif // !NO_LOGGING

	InTickFunctionComponent.TickFunction.Get().OwningWorld = InWorld;
	InTickFunctionComponent.TickFunction.Get().RegisterTickFunction(GetWorld()->PersistentLevel);

	UE_LOGFMT(LogFlecsWorld, Verbose,
		"Registered Tick Function for Entity {EntityIdentifier}",
		EntityHandle.HasName() ? EntityHandle.GetName() : EntityHandle.ToString());
}

void UFlecsRemoveTickFunctionObserver::BuildObserver(const TSolidNotNull<UFlecsWorld*> InWorld,
	TFlecsObserverBuilder<>& InOutBuilder) const
{
	InOutBuilder
		.Event(flecs::OnRemove)
		.With<FFlecsTickFunctionComponent&>() // 0
		.YieldExisting();
}

void UFlecsRemoveTickFunctionObserver::EachIterator(const TSolidNotNull<UFlecsWorld*> InWorld, flecs::iter& InIterator,
	const FFlecsId InIndex)
{
	const FFlecsTickFunctionComponent& InTickFunctionComponent = InIterator.field_at<FFlecsTickFunctionComponent>(0, InIndex);
	
	solid_checkf(InTickFunctionComponent.TickFunction.IsValid(),
				TEXT("FFlecsTickFunctionComponent has invalid TickFunction"));

#if !NO_LOGGING
	const FFlecsEntityHandle EntityHandle = InIterator.entity(InIndex);
#endif // !NO_LOGGING
			
	InTickFunctionComponent.TickFunction.Get().UnRegisterTickFunction();

	UE_LOGFMT(LogFlecsWorld, Verbose,
		"Unregistered Tick Function for Entity {EntityIdentifier}",
		EntityHandle.HasName() ? EntityHandle.GetName() : EntityHandle.ToString());
}
