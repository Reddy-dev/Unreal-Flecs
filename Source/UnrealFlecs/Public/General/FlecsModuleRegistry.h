// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Types/SolidNotNull.h"

class UFlecsWorld;

namespace UE::Flecs
{
	struct UNREALFLECS_API FFlecsModuleRegistry
	{
		static FFlecsModuleRegistry& Get();

	public:
		void RegisterUnrealFlecsModule(const FName& ModuleName);
		
		void InitializeRegisteredModules(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld) const;
		
	private:
		TSet<FName> RegisteredModules;
		
	}; // struct FFlecsModuleRegistry
	
} // namespace UE::Flecs

