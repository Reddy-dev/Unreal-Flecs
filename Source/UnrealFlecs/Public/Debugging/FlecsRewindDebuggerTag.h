// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsRewindDebuggerTag.generated.h"

/**
 * Opts a Flecs entity into passive state capture for the Rewind Debugger.
 *
 * Capture also requires the per-project Flecs Rewind Debugger setting and an
 * active Rewind Debugger recording. Scrubbing captured data never mutates the
 * live Flecs world.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsRewindDebuggerTag
{
	GENERATED_BODY()
}; // struct FFlecsRewindDebuggerTag

template <>
struct TFlecsComponentTraits<FFlecsRewindDebuggerTag>
	: public TFlecsComponentTraitsBase<FFlecsRewindDebuggerTag>
{
}; // struct TFlecsComponentTraits<FFlecsRewindDebuggerTag>
