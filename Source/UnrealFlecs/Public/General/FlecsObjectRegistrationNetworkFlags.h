// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "FlecsObjectRegistrationNetworkFlags.generated.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EFlecsObjectRegistrationNetworkFlags : uint8
{
	None = 0 UMETA(Hidden),
	Standalone = 1 << 0,
	Server = 1 << 1,
	Client = 1 << 2,
	//Editor = 1 << 3,
	//EditorWorld = 1 << 4,
	All= Standalone | Server | Client UMETA(Hidden),
}; // enum class EFlecsObjectRegistrationNetworkFlags
ENUM_CLASS_FLAGS(EFlecsObjectRegistrationNetworkFlags);

namespace UE::Flecs::Net
{
	NO_DISCARD UNREALFLECS_API EFlecsObjectRegistrationNetworkFlags GetFlagsForWorld(const TSolidNotNull<const UWorld*> InWorld);
	NO_DISCARD UNREALFLECS_API bool ShouldRegisterInWorld(const TSolidNotNull<const UWorld*> InWorld, const EFlecsObjectRegistrationNetworkFlags InFlags);
} // namespace UE::Flecs::Net
