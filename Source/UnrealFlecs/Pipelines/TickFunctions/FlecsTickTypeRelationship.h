// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsTickTypeRelationship.generated.h"

USTRUCT()
struct UNREALFLECS_API FFlecsTickTypeRelationship
{
	GENERATED_BODY()
}; // struct FFlecsTickTypeRelationship

REGISTER_FLECS_COMPONENT(FFlecsTickTypeRelationship,
	[](flecs::world InWorld, const FFlecsComponentHandle& InComponent)
	{
		InComponent
			.Add(flecs::Relationship);
	});