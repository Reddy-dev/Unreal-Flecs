// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/FlecsQueryBuilder.h"

#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryBuilder)

FFlecsQueryBuilder::FFlecsQueryBuilder(const TSolidNotNull<const UFlecsWorld*> InWorld)
{
	World = InWorld;
}
