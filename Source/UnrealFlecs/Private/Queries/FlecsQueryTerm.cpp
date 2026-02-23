// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/FlecsQueryTerm.h"

#include "Queries/FlecsQueryBuilderView.h"
#include "Queries/Generator/FlecsQueryGeneratorInputType.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryTerm)

void FFlecsQueryTerm::ApplyToQueryBuilder(const TSolidNotNull<const UFlecsWorld*> InWorld,
	FFlecsQueryBuilderView& InQueryBuilder) const
{
	FFlecsTermRefAtom_Internal TermRef = Input.GetFirstTermRef(InWorld);
	InQueryBuilder.term();
	if (TermRef.IsType<FFlecsId>())
	{
		InQueryBuilder.first(TermRef.Get<FFlecsId>());
	}
	else if (TermRef.IsType<char*>())
	{
		InQueryBuilder.first(TermRef.Get<char*>());
	}
	else UNLIKELY_ATTRIBUTE
	{
		UNREACHABLE
	}
	
	if (Input.IsPair())
	{
		FFlecsTermRefAtom_Internal SecondTermRef = Input.GetSecondTermRef(InWorld);
		if (SecondTermRef.IsType<FFlecsId>())
		{
			InQueryBuilder.second(SecondTermRef.Get<FFlecsId>());
		}
		else if (SecondTermRef.IsType<char*>())
		{
			InQueryBuilder.second(SecondTermRef.Get<char*>());
		}
		else UNLIKELY_ATTRIBUTE
		{
			UNREACHABLE
		}
	}
}
