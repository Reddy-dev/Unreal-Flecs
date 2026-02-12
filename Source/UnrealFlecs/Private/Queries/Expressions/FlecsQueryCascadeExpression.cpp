// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/Expressions/FlecsQueryCascadeExpression.h"

#include "Entities/FlecsId.h"
#include "Queries/Generator/FlecsQueryGeneratorInputType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryCascadeExpression)

FFlecsQueryCascadeExpression::FFlecsQueryCascadeExpression() : Super(false /* bInAllowsChildExpressions */)
{
}

void FFlecsQueryCascadeExpression::Apply(const TSolidNotNull<const UFlecsWorld*> InWorld,
	flecs::query_builder<>& InQueryBuilder) const
{
	if (Traversal.IsSet())
	{
		FFlecsId TraversalId;
		
		const TInstancedStruct<FFlecsQueryGeneratorInputType>& TraversalInput = Traversal.GetValue();
		if LIKELY_IF(TraversalInput.Get().ReturnType == EFlecsQueryGeneratorReturnType::FlecsId)
		{
			TraversalId = TraversalInput.Get<FFlecsQueryGeneratorInputType>().GetFlecsIdOutput(InWorld);
		}
		else UNLIKELY_ATTRIBUTE
		{
			solid_checkf(false, TEXT("Unsupported Up traversal input type return type"));
			return;
		}
		
		InQueryBuilder.cascade(TraversalId);
	}
	else
	{
		InQueryBuilder.cascade();
	}
}
