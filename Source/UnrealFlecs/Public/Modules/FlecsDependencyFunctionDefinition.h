// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include <functional>

#include "CoreMinimal.h"
#include "Entities/FlecsEntityHandle.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "FlecsDependencyFunctionDefinition.generated.h"

USTRUCT()
struct FFlecsDependencyFunctionDefinition
{
	GENERATED_BODY()
	
public:
	using FDependencyFunctionType = std::function<void(UObject*, const UFlecsWorld*, FFlecsEntityHandle)>;
	
	FDependencyFunctionType Function;

	void Call(const TSolidNotNull<UObject*> InDependencyObject,
						const TSolidNotNull<const UFlecsWorld*> InWorld,
						const FFlecsEntityHandle& InDependencyEntity) const
	{
		solid_check(Function);

		std::invoke(Function, InDependencyObject, InWorld, InDependencyEntity);
	}
	
}; // struct FFlecsDependencyFunctionDefinition
