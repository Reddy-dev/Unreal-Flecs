// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/EngineBaseTypes.h"

#include "FlecsTickingGroup.generated.h"

// @TODO: add docs

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EFlecsTickingGroup : uint8
{
	PrePhysics = 1 << 0 UMETA(DisplayName = "Pre Physics"),
	DuringPhysics = 1 << 1 UMETA(DisplayName = "During Physics"),
	EndPhysics = 1 << 2 UMETA(DisplayName = "End Physics"),
	PostPhysics = 1 << 3 UMETA(DisplayName = "Post Physics"),
	PostUpdateWork = 1 << 4 UMETA(DisplayName = "Post Update Work"),
	
}; // enum class EUnrealFlecsTickingGroup

ENUM_CLASS_FLAGS(EFlecsTickingGroup);

bool IsConvertibleToFlecsTickingGroup(const ETickingGroup InEngineTickingGroup);

ETickingGroup ConvertFlecsTickingGroupToEngineTickingGroup(const EFlecsTickingGroup InFlecsTickingGroup);
EFlecsTickingGroup ConvertEngineTickingGroupToFlecsTickingGroup(const ETickingGroup InEngineTickingGroup);

