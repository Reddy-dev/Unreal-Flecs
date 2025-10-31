// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Properties/FlecsComponentProperties.h"

#include "UnrealFlecsPhaseTag.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FUnrealFlecsPhaseTag
{
	GENERATED_BODY()
}; // struct FUnrealFlecsPhaseTag

REGISTER_FLECS_COMPONENT(FUnrealFlecsPhaseTag,
	[](flecs::world InWorld, const FFlecsComponentHandle& InComponentHandle)
	{
		InComponentHandle
			.AddPair(flecs::With, flecs::Phase);
	});
