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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	FName ObjectPrioritizerName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	FName FilterName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	FName ShardSelectorName = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	bool bAlwaysRelevant = false;
	
	// @TODO: make it only flecs components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	TArray<TInstancedStruct<FFlecsReplicationProfileParamsBase>> ParameterComponents;

	NO_DISCARD bool operator==(const FFlecsReplicationProfile& Other) const
	{
		return ObjectPrioritizerName == Other.ObjectPrioritizerName
			&& FilterName == Other.FilterName
			&& ShardSelectorName == Other.ShardSelectorName;
	}

	NO_DISCARD bool operator!=(const FFlecsReplicationProfile& Other) const
	{
		return !(*this == Other);
	}

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
