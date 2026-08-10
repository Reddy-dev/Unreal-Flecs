// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsViewerTrackerSingleton.generated.h"

USTRUCT()
struct UNREALFLECSGAMEFRAMEWORK_API FFlecsViewerTrackerSingleton
{
	GENERATED_BODY()

public:
	
	TSortedMap<FName, FFlecsEntityView, FDefaultAllocator, FNameFastLess> StreamSourceViewers;
	TSortedMap<TObjectKey<const APlayerController>, FFlecsEntityView> PCViewers;
	TSortedMap<TObjectKey<const AActor>, FFlecsEntityView> ActorViewers;

}; // struct FFlecsViewerTrackerSingleton

template <>
struct TFlecsComponentTraits<FFlecsViewerTrackerSingleton> : public TFlecsComponentTraitsBase<FFlecsViewerTrackerSingleton>
{
	static constexpr bool Singleton = true;
}; // struct TFlecsComponentTraits<FFlecsViewerTrackerSingleton>
