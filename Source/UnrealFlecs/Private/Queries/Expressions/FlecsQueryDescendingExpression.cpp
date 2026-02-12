// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/Expressions/FlecsQueryDescendingExpression.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryDescendingExpression)

FFlecsQueryDescendingExpression::FFlecsQueryDescendingExpression() : Super(false /* bInAllowsChildExpressions */)
{
}

void FFlecsQueryDescendingExpression::Apply(const TSolidNotNull<const UFlecsWorld*> InWorld,
                                            flecs::query_builder<>& InQueryBuilder) const
{
	InQueryBuilder.desc();
}
