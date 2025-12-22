// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsDefaultGameLoop.h"

#include "Logs/FlecsCategories.h"

#include "FlecsOutsideMainLoopTag.h"
#include "Containers/AnsiString.h"
#include "TickFunctions/FlecsTickTypeNativeTags.h"

#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsDefaultGameLoop)

static NO_DISCARD FORCEINLINE int flecs_entity_compare(
	const ecs_entity_t e1,
	MAYBE_UNUSED const void* ptr1,
	const ecs_entity_t e2,
	MAYBE_UNUSED const void* ptr2)
{
	return (e1 > e2) - (e1 < e2);
}

#ifdef FLECS_ENABLE_SYSTEM_PRIORITY

static NO_DISCARD FORCEINLINE int flecs_priority_compare(
	const flecs::entity_t InEntityA,
	const flecs::SystemPriority* InPtrA,
	const flecs::entity_t InEntityB,
	const flecs::SystemPriority* InPtrB) 
{
	solid_check(InPtrA);
	solid_check(InPtrB);
	
	if (InPtrA->value == InPtrB->value)
	{
		return flecs_entity_compare(InEntityA, InPtrA, InEntityB, InPtrB);
	}
	else // lower value has higher priority
	{
		return InPtrA->value >= InPtrB->value ? 1 : -1;
	}
}

#endif // FLECS_ENABLE_SYSTEM_PRIORITY

void UFlecsDefaultGameLoop::InitializeGameLoop(TSolidNotNull<UFlecsWorld*> InWorld, const FFlecsEntityHandle& InGameLoopEntity)
{
	MainLoopPipeline = InWorld->CreatePipeline()
		.with(flecs::System)
		.with(flecs::Phase).cascade(flecs::DependsOn)
		.without(flecs::Disabled).up(flecs::DependsOn)
		.without(flecs::Disabled).up(flecs::ChildOf)
		#ifdef FLECS_ENABLE_SYSTEM_PRIORITY
		.with<flecs::SystemPriority>()
		#endif // FLECS_ENABLE_SYSTEM_PRIORITY
		.without<FFlecsOutsideMainLoopTag>()
		.without<FFlecsOutsideMainLoopTag>().up(flecs::DependsOn)
		.without<FFlecsOutsideMainLoopTag>().up(flecs::ChildOf)
		#ifdef FLECS_ENABLE_SYSTEM_PRIORITY
		.order_by<flecs::SystemPriority>(flecs_priority_compare)
		#else // FLECS_ENABLE_SYSTEM_PRIORITY
		.order_by(flecs_entity_compare)
		#endif // FLECS_ENABLE_SYSTEM_PRIORITY
		//.with(InWorld->GetTagEntity(FlecsTickType_MainLoop))
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
		World->RunPipeline(PrePhysicsPipeline, DeltaTime);
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
	return { FlecsTickType_MainLoop, FlecsTickType_PrePhysics, FlecsTickType_DuringPhysics, FlecsTickType_PostPhysics, FlecsTickType_PostUpdateWork };
}

FFlecsEntityHandle UFlecsDefaultGameLoop::CreatePipelineForTickType(const FGameplayTag& InTickType,
	TSolidNotNull<UFlecsWorld*> InWorld) const
{
	auto MakeBasePipeline = [this, InWorld]() -> flecs::pipeline_builder<>
	{
		return InWorld->CreatePipeline()
			.with(flecs::System)
			.with(flecs::Phase).cascade(flecs::DependsOn)
			.without(flecs::Disabled).up(flecs::DependsOn)
			.without(flecs::Disabled).up(flecs::ChildOf)
			#ifdef FLECS_ENABLE_SYSTEM_PRIORITY
			.with<flecs::SystemPriority>()
			#endif // FLECS_ENABLE_SYSTEM_PRIORITY
			.without<FFlecsOutsideMainLoopTag>()
			.without<FFlecsOutsideMainLoopTag>().up(flecs::DependsOn)
			.without<FFlecsOutsideMainLoopTag>().up(flecs::ChildOf);
	};

	FFlecsEntityHandle ResultPipeline;

	flecs::pipeline_builder<> PipelineBuilder = MakeBasePipeline();

	#ifdef FLECS_ENABLE_SYSTEM_PRIORITY
	PipelineBuilder
			.with<flecs::SystemPriority>()
		   .order_by<flecs::SystemPriority>(flecs_priority_compare);
	#else
		PipelineBuilder.order_by(flecs_entity_compare);
	#endif

	const FString PipelineName = FString::Printf(TEXT("%s_Pipeline"), *InTickType.ToString());

	ResultPipeline = PipelineBuilder
		.with(InWorld->GetTagEntity(InTickType))
		.build()
		.set_name(StringCast<char>(*PipelineName).Get());

	return ResultPipeline;
}

