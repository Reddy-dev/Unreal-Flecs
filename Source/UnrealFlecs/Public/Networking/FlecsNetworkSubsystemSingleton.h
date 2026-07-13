// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "General/FlecsSubsystemSingletonBase.h"

#include "FlecsNetworkSubsystemSingleton.generated.h"

class UFlecsAbstractWorldSubsystem;

/**
 * Flecs singleton that exposes the owning UFlecsNetworkWorldSubsystem to
 * Flecs observers. It is installed when the Flecs world initializes.
 */
USTRUCT()
struct FFlecsNetworkSubsystemSingleton : public FFlecsSubsystemSingletonBase
{
	GENERATED_BODY()
	
	using Super::Super;
	
}; // struct FFlecsNetworkSubsystemSingleton

template <>
struct TFlecsComponentTraits<FFlecsNetworkSubsystemSingleton> : public TFlecsComponentTraitsBase<FFlecsNetworkSubsystemSingleton>
{
	static constexpr bool Singleton = true;
}; // struct TFlecsComponentTraits<FNetworkSubsystemSingleton>
