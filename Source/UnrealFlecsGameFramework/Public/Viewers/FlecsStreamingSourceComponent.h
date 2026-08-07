// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsStreamingSourceComponent.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECSGAMEFRAMEWORK_API FFlecsStreamingSourceComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FName StreamingSourceName;
	
}; // struct FFlecsStreamingSourceComponent

template <>
struct TFlecsComponentTraits<FFlecsStreamingSourceComponent> : public TFlecsComponentTraitsBase<FFlecsStreamingSourceComponent>
{
}; // struct TFlecsComponentTraits<FFlecsStreamingSourceComponent>
