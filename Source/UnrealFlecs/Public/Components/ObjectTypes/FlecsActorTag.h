// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FFlecsUObjectTag.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsActorTag.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsActorTag : public FFlecsUObjectTag
{
	GENERATED_BODY()
}; // struct FFlecsActorTag

template <>
struct TFlecsComponentTraits<FFlecsActorTag> : public TFlecsComponentTraitsBase<FFlecsActorTag>
{
	using InheritsFrom = FFlecsUObjectTag;
}; // struct TFlecsComponentTraits<FFlecsActorTag>