// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsTickFunctionPrerequisite.generated.h"

USTRUCT()
struct UNREALFLECS_API FFlecsTickFunctionPrerequisite
{
	GENERATED_BODY()
}; // struct FFlecsTickFunctionPrerequisite

REGISTER_FLECS_COMPONENT(FFlecsTickFunctionPrerequisite,
	[](flecs::world InWorld, const FFlecsComponentHandle& InComponent)
	{
		InComponent
			.Add(flecs::Relationship)
			.Add(flecs::Acyclic)
			.Add(flecs::DontFragment);
	});

