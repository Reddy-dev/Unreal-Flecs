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
	
	TMap<FName, FFlecsEntityView> StreamSourceViewers;
	TMap<TWeakObjectPtr<const APlayerController>, FFlecsEntityView> PCViewers;
	TMap<TWeakObjectPtr<const AActor>, FFlecsEntityView> ActorViewers;

}; // struct FFlecsViewerTrackerSingleton

template <>
struct TFlecsComponentTraits<FFlecsViewerTrackerSingleton> : public TFlecsComponentTraitsBase<FFlecsViewerTrackerSingleton>
{
	static constexpr bool Singleton = true;
}; // struct TFlecsComponentTraits<FFlecsViewerTrackerSingleton>
