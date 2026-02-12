// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/Expressions/FlecsQueryGroupByExpression.h"

#include "Entities/FlecsId.h"
#include "Queries/Generator/FlecsQueryGeneratorInputType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryGroupByExpression)

FFlecsQueryGroupByExpression::FFlecsQueryGroupByExpression() : Super(true /* bInAllowsChildExpressions */)
{
}

void FFlecsQueryGroupByExpression::Apply(const TSolidNotNull<const UFlecsWorld*> InWorld,
	flecs::query_builder<>& InQueryBuilder) const
{
	FFlecsId GroupByComponentId;
	
	if LIKELY_IF(GroupByInput.Get().ReturnType == EFlecsQueryGeneratorReturnType::FlecsId)
	{
		GroupByComponentId = GroupByInput.Get<FFlecsQueryGeneratorInputType>().GetFlecsIdOutput(InWorld);
	}
	else
	{
		solid_checkf(false, TEXT("Unsupported OrderBy input type return type"));
	}
	
	if (GroupByCallbackDefinition.IsSet())
	{
		//InQueryBuilder.group_by(GroupByCallbackDefinition->GetGroupByFunction(), GroupByInput.GetMutableMemory());
	}
	else
	{
		InQueryBuilder.group_by(GroupByComponentId);
	}
	
	Super::Apply(InWorld, InQueryBuilder);
}
