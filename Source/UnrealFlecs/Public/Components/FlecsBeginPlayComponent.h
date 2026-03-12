// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsBeginPlayComponent.generated.h"

/**
 * @brief Component type that represents if the World has begun play.
 * Can be found in the World entity.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsBeginPlayComponent
{
	GENERATED_BODY()
}; // struct FFlecsBeginPlayComponent

REGISTER_FLECS_COMPONENT(FFlecsBeginPlayComponent);
