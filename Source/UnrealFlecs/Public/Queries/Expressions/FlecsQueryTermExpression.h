// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Queries/Enums/FlecsQueryOperators.h"
#include "FlecsQueryExpression.h"
#include "Queries/FlecsQueryTerm.h"

#include "FlecsQueryTermExpression.generated.h"

USTRUCT(BlueprintType, meta = (DisplayName = "Term Query"))
struct UNREALFLECS_API FFlecsQueryTermExpression : public FFlecsQueryExpression
{
	GENERATED_BODY()

public:
	FFlecsQueryTermExpression();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	FFlecsQueryTerm Term;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Query")
	EFlecsQueryOperator Operator = EFlecsQueryOperator::Default;

	virtual void Apply(const TSolidNotNull<const UFlecsWorld*> InWorld, flecs::query_builder<>& InQueryBuilder) const override;
	
}; // struct FFlecsQueryTermExpression
