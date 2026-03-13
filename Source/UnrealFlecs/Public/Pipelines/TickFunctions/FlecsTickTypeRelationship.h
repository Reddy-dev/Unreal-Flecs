// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsTickTypeRelationship.generated.h"

USTRUCT()
struct UNREALFLECS_API FFlecsTickTypeRelationship
{
	GENERATED_BODY()
}; // struct FFlecsTickTypeRelationship

template <>
struct TFlecsComponentTraits<FFlecsTickTypeRelationship> : public TFlecsComponentTraitsBase<FFlecsTickTypeRelationship>
{
	static constexpr bool Relationship = true;
}; // struct TFlecsComponentTraits<FFlecsTickTypeRelationship>