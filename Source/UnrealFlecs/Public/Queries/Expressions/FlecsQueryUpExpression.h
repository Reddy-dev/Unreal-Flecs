// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsQueryExpression.h"

#include "FlecsQueryUpExpression.generated.h"

struct FFlecsQueryGeneratorInputType;

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsQueryUpExpression : public FFlecsQueryExpression
{
	GENERATED_BODY()
	
public:
	FFlecsQueryUpExpression();
	
	virtual void Apply(const TSolidNotNull<const UFlecsWorld*> InWorld, flecs::query_builder<>& InQueryBuilder) const override;
	
	UPROPERTY(EditAnywhere)
	TOptional<TInstancedStruct<FFlecsQueryGeneratorInputType>> Traversal;
	
}; // struct FFlecsQueryUpExpression
