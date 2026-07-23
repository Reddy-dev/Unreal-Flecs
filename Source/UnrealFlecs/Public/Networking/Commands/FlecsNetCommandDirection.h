// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsNetCommandDirection.generated.h"

UENUM(BlueprintType)
enum class EFlecsNetCommandDirection : uint8
{
	ServerToClient,
	ClientToServer,
	Bidirectional
}; // enum class EFlecsNetCommandDirection
