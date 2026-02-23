// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/Expressions/FlecsQueryTermExpression.h"

#include "Queries/FlecsQueryBuilderView.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryTermExpression)

FFlecsQueryTermExpression::FFlecsQueryTermExpression() : Super(true /* bInAllowsChildExpressions */)
{
}

void FFlecsQueryTermExpression::Apply(TSolidNotNull<const UFlecsWorld*> InWorld, FFlecsQueryBuilderView& InQueryBuilder) const
{
	Term.ApplyToQueryBuilder(InWorld, InQueryBuilder);
	
	InQueryBuilder.oper(ToFlecsOperator(Operator));
	
	if (Source.IsValid())
	{
		FFlecsTermRefAtom_Internal SourceAtom = Source.GetFirstTermRef(InWorld);
		if (SourceAtom.IsType<FFlecsId>())
		{
			InQueryBuilder.src(SourceAtom.Get<FFlecsId>());
		}
		else if (SourceAtom.IsType<char*>())
		{
			InQueryBuilder.src(SourceAtom.Get<char*>());
		}
		else UNLIKELY_ATTRIBUTE
		{
			UNREACHABLE
		}
	}
	
	if (bStage)
	{
		InQueryBuilder.inout_stage(ToFlecsInOut(InOut));
	}
	else 
	{
		InQueryBuilder.inout(ToFlecsInOut(InOut));
	}
	
	Super::Apply(InWorld, InQueryBuilder);
}