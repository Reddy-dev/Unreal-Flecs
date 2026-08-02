// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsNetDirtyTag.generated.h"

USTRUCT(BlueprintType)
struct FFlecsNetDirtyTag
{
	GENERATED_BODY()
	
	static constexpr bool DontFragment = true;
	
}; // struct FFlecsNetDirtyTag

template <>
struct TFlecsComponentTraits<FFlecsNetDirtyTag> : public TFlecsComponentTraitsBase<FFlecsNetDirtyTag>
{
	static constexpr bool DontFragment = true;
}; // struct TFlecsComponentTraits<FFlecsNetDirtyTag>


