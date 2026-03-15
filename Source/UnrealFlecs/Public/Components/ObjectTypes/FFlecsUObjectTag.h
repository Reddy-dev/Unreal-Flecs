// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FFlecsUObjectTag.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsUObjectTag
{
	GENERATED_BODY()
}; // struct FFlecsUObjectTag

template <>
struct TFlecsComponentTraits<FFlecsUObjectTag> : public TFlecsComponentTraitsBase<FFlecsUObjectTag>
{
	static constexpr bool Target = true;
}; // struct TFlecsComponentTraits<FFlecsUObjectTag>


