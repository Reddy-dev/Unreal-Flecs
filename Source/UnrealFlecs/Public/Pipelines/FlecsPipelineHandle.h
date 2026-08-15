// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Entities/FlecsEntityHandle.h"

#include "FlecsPipelineDefinition.h"

#include "FlecsPipelineHandle.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsPipelineHandle : public FFlecsEntityHandle
{
	GENERATED_BODY()
	
public:
	using FFlecsEntityHandle::FFlecsEntityHandle;
	
	FFlecsPipelineHandle(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, 
		const FFlecsPipelineDefinition& InPipelineBuilder, const FString& InPipelineName);
	
	
	
}; // struct FFlecsPipelineHandle