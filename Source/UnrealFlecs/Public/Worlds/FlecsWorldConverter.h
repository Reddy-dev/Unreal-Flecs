// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"


#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

class UFlecsWorld;
class UFlecsWorldInterfaceObject;

namespace UE::Flecs
{
	UNREALFLECS_API NO_DISCARD TSolidNotNull<UFlecsWorldInterfaceObject*> ToUnrealFlecsWorldInterface(const flecs::world& InWorld);
	
	/**
	 * @brief Converts a flecs::world to its corresponding UFlecsWorld
	 * @param InWorld The flecs::world to convert
	 * @return The corresponding UFlecsWorld
	 */
	UNREALFLECS_API NO_DISCARD TSolidNotNull<UFlecsWorld*> ToUnrealFlecsWorld(const flecs::world& InWorld);
	
} // namespace UE::Flecs