// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UnrealFlecsRegistrationScopeType.generated.h"

UENUM()
enum class EUnrealFlecsRegistrationScopeType : uint8
{
	None,
	Module,
	Plugin,
	CustomNameIdentifier,
	CustomSymbolIdentifier
}; // enum class EUnrealFlecsRegistrationScopeType
