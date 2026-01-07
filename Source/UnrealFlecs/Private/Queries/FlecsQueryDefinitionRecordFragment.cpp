// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/FlecsQueryDefinitionRecordFragment.h"

#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryDefinitionRecordFragment)

void FFlecsQueryDefinitionRecordFragment::PreApplyRecordToEntity(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld,
	const FFlecsEntityHandle& InEntityHandle) const
{
	flecs::query_builder<> QueryBuilder(InFlecsWorld->World, InEntityHandle);
	QueryDefinition.Apply(InFlecsWorld, QueryBuilder);
	QueryBuilder.build();
}

void FFlecsQueryDefinitionRecordFragment::PostApplyRecordToEntity(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld,
	const FFlecsEntityHandle& InEntityHandle) const
{
	
}
