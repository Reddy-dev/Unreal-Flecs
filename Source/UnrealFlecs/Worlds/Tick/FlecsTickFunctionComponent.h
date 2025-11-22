// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FlecsWorldTickFunction.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsTickFunctionComponent.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsTickFunctionComponent
{
	GENERATED_BODY()

	UPROPERTY()
	FFlecsWorldTickFunction TickFunction;
	
}; // struct FFlecsTickFunctionComponent

REGISTER_FLECS_COMPONENT(FFlecsTickFunctionComponent,
	[](flecs::world InWorld, const FFlecsComponentHandle& InComponent)
	{
		InComponent
			.Add(flecs::Relationship)
			.Add(flecs::Acyclic);
	});

