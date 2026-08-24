// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once


#include "Properties/FlecsComponentProperties.h"

#include "FlecsPredictedEntityTag.generated.h"

// @TODO: Not implemented
USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsPredictedEntityTag
{
	GENERATED_BODY()
}; // struct FFlecsPredictedEntityTag

template <>
struct TFlecsComponentTraits<FFlecsPredictedEntityTag> : public TFlecsComponentTraitsBase<FFlecsPredictedEntityTag>
{
}; // struct TFlecsComponentTraits<FFlecsPredictedEntityTag>
