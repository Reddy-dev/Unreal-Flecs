// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsReplicationProfile.generated.h"

struct FFlecsReplicationProfileParamsBase;

/** Flecs-owned policy values inherited by replicated entities through IsA. */
USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsReplicationProfile
{
	GENERATED_BODY()
	
	// @TODO: make it only flecs components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	TArray<TInstancedStruct<FFlecsReplicationProfileParamsBase>> ParameterComponents;

}; // struct FFlecsReplicationProfile

template <>
struct TFlecsComponentTraits<FFlecsReplicationProfile> : public TFlecsComponentTraitsBase<FFlecsReplicationProfile>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Inherit;
}; // struct TFlecsComponentTraits<FFlecsReplicationProfile>

/** Identifies a Flecs entity as a replication profile prefab. */
USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsReplicationProfileTag
{
	GENERATED_BODY()
}; // struct FFlecsReplicationProfileTag

template <>
struct TFlecsComponentTraits<FFlecsReplicationProfileTag> : public TFlecsComponentTraitsBase<FFlecsReplicationProfileTag>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::DontInherit;
}; // struct TFlecsComponentTraits<FFlecsReplicationProfileTag>
