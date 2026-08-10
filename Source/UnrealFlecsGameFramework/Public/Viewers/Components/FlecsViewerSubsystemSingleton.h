// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "General/FlecsSubsystemSingletonBase.h"

#include "FlecsViewerSubsystemSingleton.generated.h"

USTRUCT()
struct UNREALFLECSGAMEFRAMEWORK_API FFlecsViewerSubsystemSingleton : public FFlecsSubsystemSingletonBase
{
	GENERATED_BODY()
	
	using Super::Super;
	
}; // struct FFlecsViewerSubsystemSingleton

template <>
struct TFlecsComponentTraits<FFlecsViewerSubsystemSingleton> : public TFlecsComponentTraitsBase<FFlecsViewerSubsystemSingleton>
{
	static constexpr bool Singleton = true;
}; // struct TFlecsComponentTraits<FFlecsViewerSubsystemSingleton>
