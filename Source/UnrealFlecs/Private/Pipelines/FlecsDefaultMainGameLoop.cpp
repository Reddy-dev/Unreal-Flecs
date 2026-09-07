// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Pipelines/FlecsDefaultMainGameLoop.h"

#include "Components/UnrealFlecsPluginTag.h"
#include "Logs/FlecsCategories.h"

#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsDefaultMainGameLoop)

static NO_DISCARD FORCEINLINE int flecs_entity_compare(
	const ecs_entity_t e1,
	const void* ptr1,
	const ecs_entity_t e2,
	const void* ptr2)
{
	return (e1 > e2) - (e1 < e2);
}

UFlecsDefaultMainGameLoop::UFlecsDefaultMainGameLoop()
{
	TickFunctionSettings.bTickEvenWhenPaused = true;
}

void UFlecsDefaultMainGameLoop::InitializeGameLoop(TSolidNotNull<UFlecsWorld*> InWorld, const FFlecsEntityHandle& InGameLoopEntity)
{
	MainLoopPipeline = InWorld->CreatePipeline("MainLoopPipeline")
		.With(flecs::System)
		.With(flecs::Phase).Cascade(flecs::DependsOn)
		.Without(flecs::Disabled).Up(flecs::DependsOn)
		.Without(flecs::Disabled).Up(flecs::ChildOf)
		//.order_by(flecs_entity_compare)
		// @TODO: .with(InWorld->GetTagEntity(FlecsTickType_MainLoop))
		.Build();

	InWorld->SetPipeline(MainLoopPipeline);
}

void UFlecsDefaultMainGameLoop::DeinitializeGameLoop(TSolidNotNull<UFlecsWorld*> InWorld,
	const FFlecsEntityHandle& InGameLoopEntity)
{
	Super::DeinitializeGameLoop(InWorld, InGameLoopEntity);
	
	//InWorld->SetPipeline(flecs::pipeline{});
	MainLoopPipeline.Destroy();
}

bool UFlecsDefaultMainGameLoop::Progress(double DeltaTime, TSolidNotNull<UFlecsWorld*> InWorld, ELevelTick InTickType,
                                         ENamedThreads::Type InCurrentThread, const FGraphEventRef& InCompletionGraphEvent)
{
	return InWorld->Progress(DeltaTime);
}

bool UFlecsDefaultMainGameLoop::IsMainLoop() const
{
	return true;
}

/*FFlecsPipelineHandle UFlecsDefaultGameLoop::CreatePipelineForTickType(const FGameplayTag& InTickType,
	TSolidNotNull<UFlecsWorld*> InWorld) const
{
	auto MakeBasePipeline = [this, InWorld](const FString& InPipelineName) -> TFlecsPipelineBuilder<>
	{
		TFlecsPipelineBuilder<> PipelineBuilder = InWorld->CreatePipeline(InPipelineName)
			.With(flecs::System)
			.Without(flecs::Disabled).Up(flecs::DependsOn)
			.Without(flecs::Disabled).Up(flecs::ChildOf);
		
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
}*/

