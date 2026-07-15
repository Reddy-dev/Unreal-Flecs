// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsStablePathTag.generated.h"

USTRUCT(BlueprintType)
struct FFlecsStablePathTag
{
	GENERATED_BODY()
}; // struct FFlecsStablePathTag

template <>
struct TFlecsComponentTraits<FFlecsStablePathTag> : public TFlecsComponentTraitsBase<FFlecsStablePathTag>
{
}; // struct TFlecsComponentTraits<FFlecsStablePathTag>