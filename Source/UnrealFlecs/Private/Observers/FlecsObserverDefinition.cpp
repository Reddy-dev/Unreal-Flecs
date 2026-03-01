// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Observers/FlecsObserverDefinition.h"

#include "Queries/FlecsQueryBuilderView.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsObserverDefinition)

void FFlecsObserverDefinition::ApplyToObserver(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld,
	flecs::observer_builder<>& InObserverBuilder) const
{
	solid_checkf(Events.Num() > 0, TEXT("Observer must have at least one event type specified."));
	solid_checkf(Events.Num() <= FLECS_EVENT_DESC_MAX, TEXT("Observer cannot have more than %d event types."), FLECS_EVENT_DESC_MAX);
	
	for (const FFlecsObserverEventInput& EventInput : Events)
	{
		EventInput.ApplyToObserver(InFlecsWorld, InObserverBuilder);
	}
	
	FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal<ecs_observer_desc_t, &ecs_observer_desc_t::query>(InFlecsWorld->World, *InObserverBuilder._internal_get_desc());
	
	QueryDefinition.Apply(InFlecsWorld, QueryBuilderView);
	
	InObserverBuilder.yield_existing(bYieldExisting);
	
	InObserverBuilder.observer_flags(Flags);
	
	InObserverBuilder._internal_get_desc()->callback = callback;
	InObserverBuilder._internal_get_desc()->run = run;
	
	InObserverBuilder._internal_get_desc()->ctx = callback_ctx;
	InObserverBuilder._internal_get_desc()->callback_ctx = callback_ctx;
	InObserverBuilder._internal_get_desc()->run_ctx = run_ctx;
	
	InObserverBuilder._internal_get_desc()->ctx_free = ctx_free;
	InObserverBuilder._internal_get_desc()->callback_ctx_free = callback_ctx_free;
	InObserverBuilder._internal_get_desc()->run_ctx_free = run_ctx_free;
}
