// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/Expressions/FlecsQueryVarExpression.h"

#include "Queries/FlecsQueryBuilderView.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryVarExpression)

FFlecsQueryVarExpression::FFlecsQueryVarExpression() : Super(false /* bInAllowsChildExpressions */)
{
}

void FFlecsQueryVarExpression::Apply(const TSolidNotNull<const UFlecsWorld*> InWorld, FFlecsQueryBuilderView& InQueryBuilder) const
{
	InQueryBuilder.var(StringCast<char>(*VarName).Get());
}
