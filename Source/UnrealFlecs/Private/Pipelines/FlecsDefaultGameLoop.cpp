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
	MainLoopPipeline = InWorld->CreatePipeline("MainLoopPipeline")
		.With(flecs::System)
		.With(flecs::Phase).Cascade(flecs::DependsOn)
		.Without(flecs::Disabled).Up(flecs::DependsOn)
		.Without(flecs::Disabled).Up(flecs::ChildOf)
		.Without<FFlecsOutsideMainLoopTag>()
		.Without<FFlecsOutsideMainLoopTag>().Up(flecs::DependsOn)
		.Without<FFlecsOutsideMainLoopTag>().Up(flecs::ChildOf)
		//.order_by(flecs_entity_compare)
		// @TODO: .with(InWorld->GetTagEntity(FlecsTickType_MainLoop))
		.Without(FlecsTickType_PrePhysics)
		.Without(FlecsTickType_DuringPhysics)
		.Without(FlecsTickType_PostPhysics)
		.Without(FlecsTickType_PostUpdateWork)
		.Build();

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

FFlecsPipelineHandle UFlecsDefaultGameLoop::CreatePipelineForTickType(const FGameplayTag& InTickType,
	TSolidNotNull<UFlecsWorld*> InWorld) const
{
	auto MakeBasePipeline = [this, InWorld](const FString& InPipelineName) -> TFlecsPipelineBuilder<>
	{
		TFlecsPipelineBuilder<> PipelineBuilder = InWorld->CreatePipeline(InPipelineName)
			.With(flecs::System)
			.Without(flecs::Disabled).Up(flecs::DependsOn)
			.Without(flecs::Disabled).Up(flecs::ChildOf)
			.Without<FFlecsOutsideMainLoopTag>()
			.Without<FFlecsOutsideMainLoopTag>().Up(flecs::DependsOn)
			.Without<FFlecsOutsideMainLoopTag>().Up(flecs::ChildOf);
		
		if (bUsePhasesInUnrealTickGroups)
		{
			PipelineBuilder
				.With(flecs::Phase).Cascade(flecs::DependsOn);
		}
		
		return PipelineBuilder;
	};

	FFlecsPipelineHandle ResultPipeline;
	
	const FString PipelineName = FString::Printf(TEXT("%s_Pipeline"), 
		*InTickType.ToString().Replace(TEXT("."), TEXT("_")));

	TFlecsPipelineBuilder<> PipelineBuilder = MakeBasePipeline(PipelineName);

	//PipelineBuilder.order_by(flecs_entity_compare);

	ResultPipeline = PipelineBuilder
		.With(InTickType)
		.Build();

	return ResultPipeline;
}

