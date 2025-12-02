// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/EngineBaseTypes.h"

#include "Properties/FlecsComponentProperties.h"
#include "FlecsTickFunction.h"

#include "FlecsTickFunctionComponent.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsTickFunctionComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TInstancedStruct<FFlecsTickFunction> TickFunction;
	
}; // struct FFlecsTickFunctionComponent

REGISTER_FLECS_COMPONENT(FFlecsTickFunctionComponent,
	[](flecs::world InWorld, const FFlecsComponentHandle& InComponent)
	{
		InComponent
			.Add(flecs::Sparse);
	});
