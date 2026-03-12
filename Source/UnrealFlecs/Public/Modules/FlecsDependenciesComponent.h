// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Entities/FlecsEntityHandle.h"
#include "Properties/FlecsComponentProperties.h"
#include "FlecsDependencyFunctionDefinition.h"

#include "FlecsDependenciesComponent.generated.h"

USTRUCT()
struct UNREALFLECS_API FFlecsSoftDependenciesComponent
{
	GENERATED_BODY()

public:
	TSortedMap<UClass*, FFlecsDependencyFunctionDefinition> DependencyFunctionPtrs;
	
}; // struct FFlecsDependenciesComponent

template <>
struct TFlecsComponentTraits<FFlecsSoftDependenciesComponent> : public TFlecsComponentTraitsBase<FFlecsSoftDependenciesComponent>
{
	static constexpr bool Sparse = true;
};

REGISTER_FLECS_COMPONENT(FFlecsSoftDependenciesComponent);
