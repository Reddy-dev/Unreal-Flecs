// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Pipelines/FlecsPipelineHandle.h"

#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldInterfaceObject.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsPipelineHandle)

FFlecsPipelineHandle::FFlecsPipelineHandle(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld,
	const FFlecsPipelineDefinition& InPipelineBuilder, const FString& InPipelineName)
{
	flecs::pipeline_builder<> Builder(InWorld->GetNativeFlecsWorld(), TCHAR_TO_UTF8(*InPipelineName));
	InPipelineBuilder.ApplyToPipeline(InWorld, Builder);
	
	Entity = Builder.build();
}

const FFlecsPipelineHandle& FFlecsPipelineHandle::RunPipeline(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld,
	const double InDeltaTime) const
{
	InFlecsWorld->RunPipeline(*this, InDeltaTime);
	return *this;
}

const FFlecsPipelineHandle& FFlecsPipelineHandle::RunPipeline(const double InDeltaTime) const
{
	if UNLIKELY_IF(!ensureAlways(!GetFlecsWorld()->IsDeferred()))
	{
		return *this;
	}
	
	if UNLIKELY_IF(!ensureAlways(!GetFlecsWorld()->IsStage()))
	{
		return *this;
	}
	
	const TSolidNotNull<const UFlecsWorld*> MainFlecsWorld = GetFlecsWorld()->GetFlecsWorld();
	return RunPipeline(MainFlecsWorld, InDeltaTime);
}
