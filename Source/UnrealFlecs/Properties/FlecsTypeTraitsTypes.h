// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsTypeTraitsTypes.generated.h"

// @TODO: Unused

UENUM(BlueprintType)
enum class EFlecsOnInstantiate : uint8
{
	Override	UMETA(DisplayName = "Override"),
	Inherit		UMETA(DisplayName = "Inherit"),
	DontInherit	UMETA(DisplayName = "Don't Inherit"),
}; // enum class EFlecsOnInstantiate

UENUM(BlueprintType)
enum class EFlecsOnDelete : uint8
{
	Remove,
	Delete,
	Panic,
}; // enum class EFlecsOnDelete
