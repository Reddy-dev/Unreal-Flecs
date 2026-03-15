// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FFlecsActorComponentTag.h"

#include "Properties/FlecsComponentProperties.h"

#include "FFlecsSceneComponentTag.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsSceneComponentTag : public FFlecsActorComponentTag
{
	GENERATED_BODY()
}; // struct FFlecsSceneComponentTag

template <>
struct TFlecsComponentTraits<FFlecsSceneComponentTag> : public TFlecsComponentTraitsBase<FFlecsSceneComponentTag>
{
	using InheritsFrom = FFlecsActorComponentTag;
}; // struct TFlecsComponentTraits<FFlecsSceneComponentTag>