// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsTickFunctionPrerequisite.generated.h"

USTRUCT()
struct UNREALFLECS_API FFlecsTickFunctionPrerequisite
{
	GENERATED_BODY()

	static constexpr bool DontFragment = true;
}; // struct FFlecsTickFunctionPrerequisite

template <>
struct TFlecsComponentTraits<FFlecsTickFunctionPrerequisite> : public TFlecsComponentTraitsBase<FFlecsTickFunctionPrerequisite>
{
	static constexpr bool Relationship = true;
	static constexpr bool Acyclic = true;
	static constexpr bool DontFragment = true;
}; // struct TFlecsComponentTraits<FFlecsTickFunctionPrerequisite>
