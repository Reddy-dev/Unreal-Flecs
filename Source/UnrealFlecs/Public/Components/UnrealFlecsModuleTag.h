// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "Properties/FlecsComponentProperties.h"

#include "UnrealFlecsModuleTag.generated.h"

USTRUCT()
struct FUnrealFlecsModuleTag
{
	GENERATED_BODY()
}; // struct FUnrealFlecsModuleTag

template <>
struct TFlecsComponentTraits<FUnrealFlecsModuleTag> : public TFlecsComponentTraitsBase<FUnrealFlecsModuleTag>
{
	static constexpr bool AutoRegister = false;
	
	static constexpr bool RegisterWithUnrealModule = false;
}; // struct TFlecsComponentTraits<FUnrealFlecsModuleTag>

