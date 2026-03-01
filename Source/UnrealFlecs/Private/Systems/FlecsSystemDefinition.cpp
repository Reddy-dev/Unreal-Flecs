// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Systems/FlecsSystemDefinition.h"

#include "Queries/FlecsQueryBuilderView.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsSystemDefinition)

void FFlecsSystemDefinition::ApplyToSystem(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld,
	flecs::system_builder<>& InSystemBuilder) const
{
	FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal<ecs_system_desc_t, &ecs_system_desc_t::query>(InFlecsWorld->World, *InSystemBuilder._internal_get_desc());
	
	QueryDefinition.Apply(InFlecsWorld, QueryBuilderView);
	
	PhaseInput.ApplyToSystemDefinition(InFlecsWorld, InSystemBuilder);
	TickSourceInput.ApplyToSystemDefinition(InFlecsWorld, InSystemBuilder);
	
	InSystemBuilder._internal_get_desc()->interval = Interval;
	InSystemBuilder._internal_get_desc()->rate = Rate;
	
#if FLECS_ENABLE_SYSTEM_PRIORITY
	InSystemBuilder._internal_get_desc()->priority = Priority;
#endif // FLECS_ENABLE_SYSTEM_PRIORITY
	
	InSystemBuilder._internal_get_desc()->multi_threaded = bMultiThreaded;
	InSystemBuilder._internal_get_desc()->immediate = bImmediate;

	InSystemBuilder._internal_get_desc()->callback = callback;
	InSystemBuilder._internal_get_desc()->run = run;
	
	InSystemBuilder._internal_get_desc()->ctx = callback_ctx;
	InSystemBuilder._internal_get_desc()->callback_ctx = callback_ctx;
	InSystemBuilder._internal_get_desc()->run_ctx = run_ctx;
	
	InSystemBuilder._internal_get_desc()->ctx_free = ctx_free;
	InSystemBuilder._internal_get_desc()->callback_ctx_free = callback_ctx_free;
	InSystemBuilder._internal_get_desc()->run_ctx_free = run_ctx_free;
}
