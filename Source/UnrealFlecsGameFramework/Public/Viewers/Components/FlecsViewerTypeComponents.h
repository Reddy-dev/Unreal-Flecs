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
struct UNREALFLECSGAMEFRAMEWORK_API FFlecsViewerRelationship
{
	GENERATED_BODY()
}; // struct FFlecsViewerRelationshipComponent

static_assert(sizeof(FFlecsViewerRelationship) == 1, "FFlecsViewerRelationship must be a 1-byte struct");
static_assert(std::is_empty_v<FFlecsViewerRelationship>, "FFlecsViewerRelationship must be an empty struct");

template <>
struct TFlecsComponentTraits<FFlecsViewerRelationship> : public TFlecsComponentTraitsBase<FFlecsViewerRelationship>
{
	static constexpr bool Exclusive = true;
}; // struct TFlecsComponentTraits<FFlecsViewerRelationshipComponent>

// @TODO: Fill out function types

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
	static constexpr bool Target = true;
}; // struct TFlecsComponentTraits<FFlecsViewerStreamingSourceComponent>

USTRUCT(BlueprintType)
struct UNREALFLECSGAMEFRAMEWORK_API FFlecsViewerPlayerComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TObjectPtr<const APlayerController> PlayerController;
	
}; // struct FFlecsViewerPlayerComponent

template <>
struct TFlecsComponentTraits<FFlecsViewerPlayerComponent> : public TFlecsComponentTraitsBase<FFlecsViewerPlayerComponent>
{
	static constexpr bool Target = true;
}; // struct TFlecsComponentTraits<FFlecsViewerPlayerComponent>

USTRUCT(BlueprintType)
struct UNREALFLECSGAMEFRAMEWORK_API FFlecsViewerActorComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TObjectPtr<const AActor> Actor;
	
}; // struct FFlecsViewerActorComponent

template <>
struct TFlecsComponentTraits<FFlecsViewerActorComponent> : public TFlecsComponentTraitsBase<FFlecsViewerActorComponent>
{
	static constexpr bool Target = true;
}; // struct TFlecsComponentTraits<FFlecsViewerActorComponent>
