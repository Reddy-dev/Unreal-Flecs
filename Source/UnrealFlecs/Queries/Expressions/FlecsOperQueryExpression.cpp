﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsOperQueryExpression.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsOperQueryExpression)

FFlecsOperQueryExpression::FFlecsOperQueryExpression() : Super(false /* bInAllowsChildExpressions */)
{
}

void FFlecsOperQueryExpression::Apply(TSolidNotNull<UFlecsWorld*> InWorld, flecs::query_builder<>& InQueryBuilder) const
{
	InQueryBuilder.oper(static_cast<flecs::oper_kind_t>(Operator));
}
