// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/Expressions/FlecsQueryCascadeExpression.h"

#include "Logs/FlecsCategories.h"
#include "Entities/FlecsId.h"
#include "Queries/FlecsQueryBuilderView.h"
#include "Queries/Generator/FlecsQueryGeneratorInputType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryCascadeExpression)

FFlecsQueryCascadeExpression::FFlecsQueryCascadeExpression() : Super(false /* bInAllowsChildExpressions */)
{
}

void FFlecsQueryCascadeExpression::Apply(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld,
	FFlecsQueryBuilderView& InQueryBuilder) const
{
	if (Traversal.IsSet())
	{
		const FFlecsQueryGeneratorInput& TraversalInput = Traversal.GetValue();
		const FFlecsTermRefAtom_Internal TraversalTermRef = TraversalInput.GetFirstTermRef(InWorld);
		
		if UNLIKELY_IF(TraversalTermRef.IsType<char*>())
		{
			UE_LOG(LogFlecsCore, Error, 
				TEXT("Traversal input for cascade expression cannot be a string. Cascade will be applied without traversal."));
			UE::Flecs::Queries::FreeTermRefAtom(TraversalTermRef);
			return;
		}
		
		InQueryBuilder.cascade(TraversalTermRef.Get<FFlecsId>());
	}
	else
	{
		InQueryBuilder.cascade();
	}
}
