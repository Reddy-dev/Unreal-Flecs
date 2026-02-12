// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsQueryExpression.h"
#include "Queries/Callbacks/FlecsOrderByCallbackDefinition.h"

#include "FlecsQueryOrderByExpression.generated.h"

struct FFlecsOrderByCallbackDefinition;
struct FFlecsQueryGeneratorInputType;

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsQueryOrderByExpression : public FFlecsQueryExpression
{
	GENERATED_BODY()
	
public:
	FFlecsQueryOrderByExpression();
	
	virtual void Apply(const TSolidNotNull<const UFlecsWorld*> InWorld, flecs::query_builder<>& InQueryBuilder) const override;
	
	UPROPERTY(EditAnywhere)
	TInstancedStruct<FFlecsQueryGeneratorInputType> OrderByInput;
	
	UPROPERTY(EditAnywhere)
	TInstancedStruct<FFlecsOrderByCallbackDefinition> OrderByCallback;
	
}; // struct FFlecsQueryOrderByExpression

// Used for CPP Lambda order by expressions
USTRUCT(NotBlueprintType)
struct UNREALFLECS_API FFlecsQueryOrderByCPPExpressionWrapper : public FFlecsQueryExpression
{
	GENERATED_BODY()
	
public:
	FFlecsQueryOrderByCPPExpressionWrapper();
	
	virtual void Apply(const TSolidNotNull<const UFlecsWorld*> InWorld, flecs::query_builder<>& InQueryBuilder) const override;
	
	UPROPERTY()
	TInstancedStruct<FFlecsQueryGeneratorInputType> OrderByInput;
	
	Unreal::Flecs::Queries::FOrderByFunctionType OrderByFunction;
	
}; // struct FFlecsQueryOrderByCPPExpressionWrapper
