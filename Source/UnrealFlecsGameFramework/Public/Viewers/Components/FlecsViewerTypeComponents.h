// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsViewerTypeComponents.generated.h"

UENUM(BlueprintType)
enum class EFlecsViewerType : uint8
{
	Player,
	Actor,
	StreamingSource
}; // enum class EFlecsViewerType

template <>
struct TFlecsComponentTraits<EFlecsViewerType> : public TFlecsComponentTraitsBase<EFlecsViewerType>
{
	static constexpr bool Exclusive = true;
}; // struct TFlecsComponentTraits<EFlecsViewerType>


USTRUCT(BlueprintType)
struct UNREALFLECSGAMEFRAMEWORK_API FFlecsViewerStreamingSourceComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FName StreamingSourceName;
	
}; // struct FFlecsViewerStreamingSourceComponent

template <>
struct TFlecsComponentTraits<FFlecsViewerStreamingSourceComponent> : public TFlecsComponentTraitsBase<FFlecsViewerStreamingSourceComponent>
{
	static constexpr Target = true;
}; // struct TFlecsComponentTraits<FFlecsViewerStreamingSourceComponent>

USTRUCT(BlueprintType)
struct UNREALFLECSGAMEFRAMEWORK_API FFlecsViewerActorComponent
{
	GENERATED_BODY()
}; // struct FFlecsViewerActorComponent

