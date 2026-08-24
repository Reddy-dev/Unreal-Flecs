// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "UnrealFlecsPluginTag.generated.h"

// Used by Registered Flecs Plugins, this will most likely be used by Game feature plugins
USTRUCT()
struct FUnrealFlecsPluginTag
{
	GENERATED_BODY()
}; // struct FUnrealFlecsPluginTag

template <>
struct TFlecsComponentTraits<FUnrealFlecsPluginTag> : public TFlecsComponentTraitsBase<FUnrealFlecsPluginTag>
{
	static constexpr bool AutoRegister = false;
	
	static constexpr bool RegisterWithUnrealModule = false;
}; // struct TFlecsComponentTraits<FUnrealFlecsPluginTag>

