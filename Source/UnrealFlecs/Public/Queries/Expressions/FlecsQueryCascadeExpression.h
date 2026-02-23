// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsQueryExpression.h"
#include "Queries/Generator/FlecsQueryGeneratorInput.h"

#include "FlecsQueryCascadeExpression.generated.h"

struct FFlecsQueryGeneratorInputType;

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsQueryCascadeExpression : public FFlecsQueryExpression
{
	GENERATED_BODY()
	
public:
	FFlecsQueryCascadeExpression();
	
	virtual void Apply(const TSolidNotNull<const UFlecsWorld*> InWorld, FFlecsQueryBuilderView& InQueryBuilder) const override;
	
	UPROPERTY(EditAnywhere, meta = (AllowPairInput = false, AllowStringInput = false))
	TOptional<FFlecsQueryGeneratorInput> Traversal;
	
}; // struct FFlecsQueryCascadeExpression
