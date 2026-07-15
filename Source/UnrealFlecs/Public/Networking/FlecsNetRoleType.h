// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsNetRoleType.generated.h"

UENUM(BlueprintType)
enum class EFlecsNetRoleType : uint8
{
	None UMETA(DisplayName = "None"),
	Authority UMETA(DisplayName = "Authority"),
	AutonomousProxy UMETA(DisplayName = "Autonomous Proxy"),
	SimulatedProxy UMETA(DisplayName = "Simulated Proxy"),
	ROLE_MAX UMETA(Hidden),
}; // enum class EFlecsNetRoleType



