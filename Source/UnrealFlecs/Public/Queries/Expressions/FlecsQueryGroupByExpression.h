// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsQueryExpression.h"

#include "FlecsQueryGroupByExpression.generated.h"

struct FFlecsGroupByCallbackDefinition;
struct FFlecsQueryGeneratorInputType;

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsQueryGroupByExpression : public FFlecsQueryExpression
{
	GENERATED_BODY()
	
public:
	FFlecsQueryGroupByExpression();
	
	virtual void Apply(const TSolidNotNull<const UFlecsWorld*> InWorld, flecs::query_builder<>& InQueryBuilder) const override;
	
	UPROPERTY(EditAnywhere)
	TInstancedStruct<FFlecsQueryGeneratorInputType> GroupByInput;
	
	UPROPERTY(EditAnywhere)
	TOptional<TInstancedStruct<FFlecsGroupByCallbackDefinition>> GroupByCallbackDefinition;
	
}; // struct FFlecsQueryGroupByExpression
