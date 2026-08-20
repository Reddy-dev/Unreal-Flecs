// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsProfileRelationshipTypes.generated.h"

USTRUCT()
struct UNREALFLECSNETWORKING_API FFlecsObjectPrioritizerRelationship
{
	GENERATED_BODY()
}; // struct FFlecsObjectPrioritizerRelationship

template <>
struct TFlecsComponentTraits<FFlecsObjectPrioritizerRelationship> : public TFlecsComponentTraitsBase<FFlecsObjectPrioritizerRelationship>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Inherit;
	
	static constexpr bool Relationship = true;
}; // struct TFlecsComponentTraits<FFlecsObjectPrioritizerRelationship>

USTRUCT()
struct UNREALFLECSNETWORKING_API FFlecsNetFilterRelationship
{
	GENERATED_BODY()
}; // struct FFlecsNetFilterRelationship

template <>
struct TFlecsComponentTraits<FFlecsNetFilterRelationship> : public TFlecsComponentTraitsBase<FFlecsNetFilterRelationship>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Inherit;
	
	static constexpr bool Relationship = true;
}; // struct TFlecsComponentTraits<FFlecsNetFilterRelationship>

USTRUCT()
struct UNREALFLECSNETWORKING_API FFlecsNetShardSelectorRelationship
{
	GENERATED_BODY()
}; // struct FFlecsNetShardSelectorRelationship


template <>
struct TFlecsComponentTraits<FFlecsNetShardSelectorRelationship> : public TFlecsComponentTraitsBase<FFlecsNetShardSelectorRelationship>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Inherit;
	
	static constexpr bool Relationship = true;
}; // struct TFlecsComponentTraits<FFlecsNetShardSelectorRelationship>

USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsNetProfileNameTarget
{
	GENERATED_BODY()
	
	static constexpr flecs::on_instantiate OnInstantiate = flecs::on_instantiate::inherit;
	
public:
	FFlecsNetProfileNameTarget();
	
	UPROPERTY(EditAnywhere)
	FName Name;
	
}; // struct FFlecsNetProfileNameTarget

template <>
struct TFlecsComponentTraits<FFlecsNetProfileNameTarget> : public TFlecsComponentTraitsBase<FFlecsNetProfileNameTarget>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Inherit;
	
	static constexpr bool Target = true;
}; // struct TFlecsComponentTraits<FFlecsNetProfileNameTarget>
