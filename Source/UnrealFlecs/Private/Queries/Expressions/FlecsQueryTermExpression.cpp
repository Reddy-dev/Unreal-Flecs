// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/Expressions/FlecsQueryTermExpression.h"

#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryTermExpression)

FFlecsQueryTermExpression::FFlecsQueryTermExpression() : Super(true /* bInAllowsChildExpressions */)
{
}

void FFlecsQueryTermExpression::Apply(TSolidNotNull<const UFlecsWorld*> InWorld, flecs::query_builder<>& InQueryBuilder) const
{
	Term.ApplyToQueryBuilder(InWorld, InQueryBuilder);
	
	InQueryBuilder.oper(ToFlecsOperator(Operator));
	
	Super::Apply(InWorld, InQueryBuilder);
}