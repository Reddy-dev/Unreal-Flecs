// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsBeginPlaySingletonComponent.generated.h"

/**
 * @brief Component type that represents if the World has begun play.
 * Can be found in the World entity.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsBeginPlaySingletonComponent
{
	GENERATED_BODY()
}; // struct FFlecsBeginPlaySingletonComponent

template <>
struct TFlecsComponentTraits<FFlecsBeginPlaySingletonComponent> : public TFlecsComponentTraitsBase<FFlecsBeginPlaySingletonComponent>
{
	static constexpr bool bIsSingleton = true;
}; // struct TFlecsComponentTraits<FFlecsBeginPlaySingletonComponent>
