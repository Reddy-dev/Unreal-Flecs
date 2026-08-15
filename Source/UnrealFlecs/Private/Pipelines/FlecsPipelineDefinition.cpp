// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Pipelines/FlecsPipelineDefinition.h"

#include "Queries/FlecsQueryBuilderView.h"
#include "Worlds/FlecsWorldInterfaceObject.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsPipelineDefinition)

void FFlecsPipelineDefinition::ApplyToPipeline(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InFlecsWorld,
	flecs::pipeline_builder<>& InPipelineBuilder) const
{
	FFlecsQueryBuilderView QueryBuilderView = MakeQueryBuilderView_Internal<ecs_pipeline_desc_t, &ecs_pipeline_desc_t::query>(
		InFlecsWorld->GetNativeFlecsWorld(), *InPipelineBuilder._internal_get_desc());
	
	QueryDefinition.Apply(InFlecsWorld, QueryBuilderView);
}
