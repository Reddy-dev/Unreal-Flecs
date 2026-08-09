// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Properties/FlecsComponentProperties.h"

#include "FlecsViewerTransformComponent.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECSGAMEFRAMEWORK_API FFlecsViewerTransformComponent
{
	GENERATED_BODY()
	// @TODO: maybe dont use LWC type?
	
public:
	UPROPERTY()
	FVector Location = FVector::ZeroVector;
	
	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;
	
}; // struct FFlecsViewerTransformComponent

static_assert(std::is_trivially_copyable_v<FFlecsViewerTransformComponent>, "FFlecsViewerTransformComponent must be trivially copyable.");

template <>
struct TFlecsComponentTraits<FFlecsViewerTransformComponent> : public TFlecsComponentTraitsBase<FFlecsViewerTransformComponent>
{
}; // struct TFlecsComponentTraits<FFlecsViewerTransformComponent>