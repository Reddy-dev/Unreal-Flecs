// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsQueryExpression.h"

#include "FlecsQueryScopeExpression.generated.h"

UENUM(BlueprintType)
enum class EFlecsQueryScopeExpressionType : uint8
{
	Open,
	Close
}; // enum class EFlecsQueryScopeExpressionType

USTRUCT(BlueprintType, DisplayName = "Scope Expression")
struct UNREALFLECS_API FFlecsQueryScopeExpression : public FFlecsQueryExpression
{
	GENERATED_BODY()
	
public:
	FFlecsQueryScopeExpression();
	
	UPROPERTY(EditAnywhere)
	EFlecsQueryScopeExpressionType ScopeType = EFlecsQueryScopeExpressionType::Open;
	
	virtual void Apply(const TSolidNotNull<const UFlecsWorld*> InWorld, flecs::query_builder<>& InQueryBuilder) const override;
	
}; // struct FFlecsQueryScopeExpression