// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Queries/FlecsQueryBuilderBase.h"
#include "FlecsPipelineDefinition.h"

template <typename TInherited>
struct TFlecsPipelineBuilderBase : public TFlecsQueryBuilderBase<TInherited>
{
public:
	FORCEINLINE FFlecsPipelineDefinition& GetPipelineDefinition() const
	{
		return this->GetSelf().GetPipelineDefinition_Impl();
	}
	
	FORCEINLINE FFlecsQueryDefinition& GetQueryDefinition_Impl() const
	{
		return const_cast<FFlecsQueryDefinition&>(GetPipelineDefinition().QueryDefinition);
	}
	
}; // struct TFlecsPipelineBuilderBase

