// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsPlayerControllerObjectTag.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECSGAMEFRAMEWORK_API FFlecsPlayerControllerObjectTag
{
	GENERATED_BODY()
}; // struct FFlecsPlayerControllerObjectTag

template <>
struct TFlecsComponentTraits<FFlecsPlayerControllerObjectTag> : public TFlecsComponentTraitsBase<FFlecsPlayerControllerObjectTag>
{
	static constexpr bool Target = true;
}; // struct TFlecsComponentTraits<FFlecsPlayerControllerObjectTag>