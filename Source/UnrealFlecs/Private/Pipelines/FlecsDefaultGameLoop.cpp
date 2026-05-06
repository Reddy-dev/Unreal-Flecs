// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Pipelines/FlecsDefaultGameLoop.h"

#include "Logs/FlecsCategories.h"

#include "Pipelines/FlecsOutsideMainLoopTag.h"
#include "Pipelines/FlecsTickTypeNativeTags.h"

#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsDefaultGameLoop)

static NO_DISCARD FORCEINLINE int flecs_entity_compare(
	const ecs_entity_t e1,
	const void* ptr1,
	const ecs_entity_t e2,
	const void* ptr2)
{
	return (e1 > e2) - (e1 < e2);
}

UFlecsDefaultGameLoop::UFlecsDefaultGameLoop()
{
}

void UFlecsDefaultGameLoop::InitializeGameLoop(TSolidNotNull<UFlecsWorld*> InWorld, const FFlecsEntityHandle& InGameLoopEntity)
{
	MainLoopPipeline = InWorld->CreatePipeline()
		.with(flecs::System)
		.with(flecs::Phase).cascade(flecs::DependsOn)
		.without(flecs::Disabled).up(flecs::DependsOn)
		.without(flecs::Disabled).up(flecs::ChildOf)
		.without<FFlecsOutsideMainLoopTag>()
		.without<FFlecsOutsideMainLoopTag>().up(flecs::DependsOn)
		.without<FFlecsOutsideMainLoopTag>().up(flecs::ChildOf)
		//.order_by(flecs_entity_compare)
		// @TODO: .with(InWorld->GetTagEntity(FlecsTickType_MainLoop))
		.without(InWorld->GetTagEntity(FlecsTickType_PrePhysics))
		.without(InWorld->GetTagEntity(FlecsTickType_DuringPhysics))
		.without(InWorld->GetTagEntity(FlecsTickType_PostPhysics))
		.without(InWorld->GetTagEntity(FlecsTickType_PostUpdateWork))
		.build()
		.set_name("MainLoopPipeline");

	InWorld->SetPipeline(MainLoopPipeline);
	
	PrePhysicsPipeline = CreatePipelineForTickType(FlecsTickType_PrePhysics, InWorld);

	DuringPhysicsPipeline = CreatePipelineForTickType(FlecsTickType_DuringPhysics, InWorld);

	PostPhysicsPipeline = CreatePipelineForTickType(FlecsTickType_PostPhysics, InWorld);

	PostUpdateWorkPipeline = CreatePipelineForTickType(FlecsTickType_PostUpdateWork, InWorld);
}

bool UFlecsDefaultGameLoop::Progress(const double DeltaTime, const FGameplayTag& InTickType, const TSolidNotNull<UFlecsWorld*> InWorld)
{
	if (InTickType == FlecsTickType_MainLoop)
	{
		return InWorld->Progress(DeltaTime);
	}
	else if (InTickType == FlecsTickType_PrePhysics)
	{
		InWorld->RunPipeline(PrePhysicsPipeline, DeltaTime);
	}
	else if (InTickType == FlecsTickType_DuringPhysics)
	{
		InWorld->RunPipeline(DuringPhysicsPipeline, DeltaTime);
	}
	else if (InTickType == FlecsTickType_PostPhysics)
	{
		InWorld->RunPipeline(PostPhysicsPipeline, DeltaTime);
	}
	else if (InTickType == FlecsTickType_PostUpdateWork)
	{
		InWorld->RunPipeline(PostUpdateWorkPipeline, DeltaTime);
	}
	else UNLIKELY_ATTRIBUTE
	{
		UE_LOGFMT(LogFlecsWorld, Warning,
			"Unknown TickTypeTag {TickTypeTag} passed to DefaultGameLoop Progress, skipping.",
			*InTickType.ToString());
		
		return false;
	}
	
	return true;
}

bool UFlecsDefaultGameLoop::IsMainLoop() const
{
	return true;
}

TArray<FGameplayTag> UFlecsDefaultGameLoop::GetTickTypeTags() const
{
	return { FlecsTickType_MainLoop, 
		FlecsTickType_PrePhysics, FlecsTickType_DuringPhysics, 
		FlecsTickType_PostPhysics, FlecsTickType_PostUpdateWork };
}

FFlecsEntityHandle UFlecsDefaultGameLoop::CreatePipelineForTickType(const FGameplayTag& InTickType,
	TSolidNotNull<UFlecsWorld*> InWorld) const
{
	auto MakeBasePipeline = [this, InWorld]() -> flecs::pipeline_builder<>
	{
		flecs::pipeline_builder<> PipelineBuilder = InWorld->CreatePipeline()
			.with(flecs::System)
			.without(flecs::Disabled).up(flecs::DependsOn)
			.without(flecs::Disabled).up(flecs::ChildOf)
			.without<FFlecsOutsideMainLoopTag>()
			.without<FFlecsOutsideMainLoopTag>().up(flecs::DependsOn)
			.without<FFlecsOutsideMainLoopTag>().up(flecs::ChildOf);
		
		if (bUsePhasesInUnrealTickGroups)
		{
			PipelineBuilder
				.with(flecs::Phase).cascade(flecs::DependsOn);
		}
		
		return PipelineBuilder;
	};

	FFlecsEntityHandle ResultPipeline;

	flecs::pipeline_builder<> PipelineBuilder = MakeBasePipeline();

	//PipelineBuilder.order_by(flecs_entity_compare);

	const FString PipelineName = FString::Printf(TEXT("%s_Pipeline"), *InTickType.ToString());

	ResultPipeline = PipelineBuilder
		.with(InWorld->GetTagEntity(InTickType))
		.build()
		.set_name(StringCast<char>(*PipelineName).Get());

	return ResultPipeline;
}

