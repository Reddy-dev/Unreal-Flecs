// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/Expressions/FlecsQueryScopeExpression.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryScopeExpression)

FFlecsQueryScopeExpression::FFlecsQueryScopeExpression() : Super(true /* bInAllowsChildExpressions */)
{
}

void FFlecsQueryScopeExpression::Apply(const TSolidNotNull<const UFlecsWorld*> InWorld,
	flecs::query_builder<>& InQueryBuilder) const
{
	if (ScopeType == EFlecsQueryScopeExpressionType::Open)
	{
		InQueryBuilder.scope_open();
	}
	else if (ScopeType == EFlecsQueryScopeExpressionType::Close)
	{
		InQueryBuilder.scope_close();
	}
	else UNLIKELY_ATTRIBUTE
	{
		UNREACHABLE
	}
	
	Super::Apply(InWorld, InQueryBuilder);
}
