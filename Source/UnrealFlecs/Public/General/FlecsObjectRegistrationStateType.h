// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsObjectRegistrationStateType.generated.h"

UENUM(BlueprintType)
enum class EFlecsObjectRegistrationStateType : uint8
{
	Active,
	Inactive,
}; // enum class EFlecsObjectRegistrationStateType
