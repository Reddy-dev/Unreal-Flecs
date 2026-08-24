// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "Entities/FlecsId.h"

#include "UnrealFlecsRegistrationScopeType.generated.h"

class UFlecsWorld;

UENUM()
enum class EUnrealFlecsRegistrationScopeType : uint8
{
	None,
	Module,
	Plugin,
	CustomNameIdentifier,
	CustomSymbolIdentifier
}; // enum class EUnrealFlecsRegistrationScopeType

namespace UE::Flecs::Registration
{
	UNREALFLECS_API NO_DISCARD FName ResolveScopeTypeName(const TSolidNotNull<const UObject*> InObject, 
		const EUnrealFlecsRegistrationScopeType InScopeType);
	
	UNREALFLECS_API NO_DISCARD FFlecsId ResolveRegistrationScopeToId(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld,
		const FName& ScopeName, const EUnrealFlecsRegistrationScopeType InScopeType);
	
} // namespace UE::Flecs::Registration
