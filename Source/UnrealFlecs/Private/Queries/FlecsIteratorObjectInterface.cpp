// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/FlecsIteratorObjectInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsIteratorObjectInterface)

void IFlecsIteratorObjectInterface::RunIterator(const TSolidNotNull<const UFlecsWorld*> InWorld, flecs::iter& InIterator)
{
	while (InIterator.next())
	{
		for (const FFlecsId Entity : InIterator)
		{
			EachIterator(InWorld, InIterator, Entity);
		}
	}
}

void IFlecsIteratorObjectInterface::EachIterator(const TSolidNotNull<const UFlecsWorld*> InWorld,
	flecs::iter& InIterator, const FFlecsId InIndex)
{
	// Empty
}
