// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsViewerPerspectiveComponent.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsViewerPerspectiveComponent
{
	GENERATED_BODY()
	
	// @TODO
	
public:
	UPROPERTY()
	float FieldOfView = 90.0f;
	
	UPROPERTY()
	float AspectRatio = 16.0f / 9.0f;
	
}; // struct FFlecsViewerPerspectiveComponent

template <>
struct TFlecsComponentTraits<FFlecsViewerPerspectiveComponent> : public TFlecsComponentTraitsBase<FFlecsViewerPerspectiveComponent>
{
}; // struct TFlecsComponentTraits<FFlecsViewerPerspectiveComponent>

