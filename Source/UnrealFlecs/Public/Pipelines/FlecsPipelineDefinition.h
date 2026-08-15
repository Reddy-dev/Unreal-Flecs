// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Queries/FlecsQueryDefinition.h"

#include "FlecsPipelineDefinition.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsPipelineDefinition
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FFlecsQueryDefinition QueryDefinition;
	
	void ApplyToPipeline(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InFlecsWorld, flecs::pipeline_builder<>& InPipelineBuilder) const;
	
}; // struct FFlecsPipelineDefinition
