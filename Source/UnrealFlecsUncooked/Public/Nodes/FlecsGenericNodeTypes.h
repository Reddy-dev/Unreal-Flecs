// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsGenericNodeTypes.generated.h"

UENUM(BlueprintType)
enum class EFlecsBlueprintGenericPinTypes : uint8
{
	Id = 0 UMETA(DisplayName = "Id"),
	ScriptStruct = 1 UMETA(DisplayName = "Struct"),
	Enum = 2 UMETA(DisplayName = "Enum"),
	EnumConstant = 3 UMETA(DisplayName = "Enum Constant"),
	GameplayTag = 4 UMETA(DisplayName = "Gameplay Tag"),
}; // EFlecsBlueprintGenericPinType

UENUM()
enum class EFlecsBlueprintSetComponentType : uint8
{
	Id = 0 UMETA(DisplayName = "Flecs Id"),
	ScriptStruct = 1 UMETA(DisplayName = "Struct"),
}; // EFlecsBlueprintSetComponentType
