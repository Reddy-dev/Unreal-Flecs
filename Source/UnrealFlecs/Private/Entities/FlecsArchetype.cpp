// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Entities/FlecsArchetype.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsArchetype)

void FFlecsArchetype::SetType(const flecs::type& InType)
{
	Type = InType;
}

void FFlecsArchetype::SetType(const flecs::type* InType)
{
	SetType(*InType);
}
