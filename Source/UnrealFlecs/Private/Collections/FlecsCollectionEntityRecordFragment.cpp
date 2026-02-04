// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Collections/FlecsCollectionEntityRecordFragment.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsCollectionEntityRecordFragment)

void FFlecsCollectionsEntityRecordFragment::PostApplyRecordToEntity(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld,
	const FFlecsEntityHandle& InEntityHandle) const
{
	for (const FFlecsCollectionInstancedReference& CollectionInstancedReference : CollectionInstancedReferences)
	{
		InEntityHandle.AddCollection(CollectionInstancedReference);
	}
}
