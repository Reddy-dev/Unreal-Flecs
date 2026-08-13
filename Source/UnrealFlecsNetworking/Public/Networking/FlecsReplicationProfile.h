// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsReplicationProfile.generated.h"

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

USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsReplicationProfileCullDistance
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	float CullDistance = 0.f;

	NO_DISCARD bool operator==(const FFlecsReplicationProfileCullDistance& Other) const
	{
		return CullDistance == Other.CullDistance;
	}

	NO_DISCARD bool operator!=(const FFlecsReplicationProfileCullDistance& Other) const
	{
		return !(*this == Other);
	}
	
}; // struct FFlecsReplicationProfileCullDistance

template <>
struct TFlecsComponentTraits<FFlecsReplicationProfileCullDistance> : public TFlecsComponentTraitsBase<FFlecsReplicationProfileCullDistance>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Inherit;
}; // struct TFlecsComponentTraits<FFlecsReplicationProfileCullDistance>

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
