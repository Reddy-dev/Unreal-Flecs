// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "EntityRecords/FlecsNamedEntityRecordFragment.h"

#include "Components/FlecsSubEntityRecordNameComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNamedEntityRecordFragment)

void FFlecsNamedEntityRecordFragment::PreApplyRecordToEntity(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InFlecsWorld,
	const FFlecsEntityHandle& InEntityHandle) const
{
	InEntityHandle.SetName(Name);
	
	if (bNameInheritedSubEntities)
	{
		InEntityHandle.Set<FFlecsSubEntityRecordNameComponent>({ .SubEntityName = Name });
	}
}