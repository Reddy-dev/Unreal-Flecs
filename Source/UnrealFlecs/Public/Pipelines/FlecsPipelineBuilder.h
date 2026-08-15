// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsPipelineBuilderBase.h"
#include "FlecsPipelineHandle.h"
#include "Queries/FlecsQueryBuilder.h"

template <typename ...TComponents>
struct TFlecsPipelineBuilder : TFlecsPipelineBuilderBase<TFlecsPipelineBuilder<TComponents...>>
{
public:
	FORCEINLINE FFlecsPipelineDefinition& GetPipelineDefinition_Impl() const
	{
		return const_cast<FFlecsPipelineDefinition&>(PipelineDefinition);
	}
	
	FORCEINLINE TFlecsPipelineBuilder(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, const FString& InOptionalName, 
		const FFlecsPipelineDefinition& InPipelineDefinition = FFlecsPipelineDefinition())
									: PipelineDefinition(InPipelineDefinition)
									, FlecsWorld(InWorld)
									, OptionalName(InOptionalName)
	{
		UE::Flecs::Queries::TAddInputTypes<TFlecsPipelineBuilder, TComponents...>::Apply(*this);
	}
	
	FORCEINLINE FFlecsPipelineHandle Build() const
	{
		solid_checkf(FlecsWorld.IsValid(), TEXT("World is not valid."));

		return FFlecsPipelineHandle(FlecsWorld.Get(), PipelineDefinition, OptionalName);
	}
	
	FFlecsPipelineDefinition PipelineDefinition;
	
	TWeakObjectPtr<const UFlecsWorldInterfaceObject> FlecsWorld;
	
	FString OptionalName;

protected:

}; // struct TFlecsPipelineBuilder

