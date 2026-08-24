// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "Properties/FlecsComponentProperties.h"

#include "FlecsStablePathTag.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsStablePathTag
{
	GENERATED_BODY()
	
	static constexpr bool DontFragment = true;
	
}; // struct FFlecsStablePathTag

template <>
struct TFlecsComponentTraits<FFlecsStablePathTag> : public TFlecsComponentTraitsBase<FFlecsStablePathTag>
{
	static constexpr bool DontFragment = true;
}; // struct TFlecsComponentTraits<FFlecsStablePathTag>
