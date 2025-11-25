// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsTickGroupRelationship.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsTickGroupRelationship
{
	GENERATED_BODY()
};

REGISTER_FLECS_COMPONENT(FFlecsTickGroupRelationship,
	[](flecs::world InWorld, const FFlecsComponentHandle& InComponent)
	{
		InComponent
			.Add(flecs::Relationship)
			.Add(flecs::Acyclic)
			.Add(flecs::DontFragment);
	});

