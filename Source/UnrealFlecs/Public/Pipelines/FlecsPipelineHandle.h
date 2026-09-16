// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "Entities/FlecsEntityHandle.h"

#include "FlecsPipelineDefinition.h"

#include "FlecsPipelineHandle.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsPipelineHandle final : public FFlecsEntityHandle
{
	GENERATED_BODY()
	
public:
	using FFlecsEntityHandle::FFlecsEntityHandle;
	
	FFlecsPipelineHandle(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, 
		const FFlecsPipelineDefinition& InPipelineBuilder, const FString& InPipelineName);
	
	const FFlecsPipelineHandle& RunPipeline(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld, 
		const double InDeltaTime = 0.0) const;
	const FFlecsPipelineHandle& RunPipeline(const double InDeltaTime = 0.0) const;
	
}; // struct FFlecsPipelineHandle