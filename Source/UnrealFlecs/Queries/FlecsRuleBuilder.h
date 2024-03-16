﻿// Solstice Games © 2024. All Rights Reserved.

#ifndef FLECS_RULE_BUILDER_H
#define FLECS_RULE_BUILDER_H

#include "CoreMinimal.h"
#include "FlecsBaseBuilder.h"
#include "FlecsRule.h"

struct FFlecsRuleBuilder : public TFlecsBaseQueryBuilder<flecs::rule_builder<>, FFlecsRuleBuilder, FFlecsRule>
{
}; // struct FFlecsRuleBuilder

#endif // FLECS_RULE_BUILDER_H