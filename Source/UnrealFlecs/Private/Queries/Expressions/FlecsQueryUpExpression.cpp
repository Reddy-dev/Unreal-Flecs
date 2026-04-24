// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/Expressions/FlecsQueryUpExpression.h"

#include "Entities/FlecsId.h"
#include "Logs/FlecsCategories.h"
#include "Queries/FlecsQueryBuilderView.h"
#include "Queries/Generator/FlecsQueryGeneratorInputType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryUpExpression)

FFlecsQueryUpExpression::FFlecsQueryUpExpression() : Super(false /* bInAllowsChildExpressions */)
{
}

void FFlecsQueryUpExpression::Apply(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld,
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
		
		InQueryBuilder.up(TraversalTermRef.Get<FFlecsId>());
	}
	else
	{
		InQueryBuilder.up();
	}
}
