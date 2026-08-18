// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsReplicationProfileParamsBase.h"

#include "FlecsReplicationProfileParamTypes.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsReplicationProfileCullDistance : public FFlecsReplicationProfileParamsBase
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
	
	virtual void ApplyToEntity(const FFlecsEntityHandle& InEntity) const override;
	
}; // struct FFlecsReplicationProfileCullDistance

USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsReplicationProfileUpdateRate : public FFlecsReplicationProfileParamsBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	float UpdateRate = 0.f;
	
	NO_DISCARD bool operator==(const FFlecsReplicationProfileUpdateRate& Other) const
	{
		return UpdateRate == Other.UpdateRate;
	}
	
	NO_DISCARD bool operator!=(const FFlecsReplicationProfileUpdateRate& Other) const
	{
		return !(*this == Other);
	}
	
	virtual void ApplyToEntity(const FFlecsEntityHandle& InEntity) const override;
	
}; // struct FFlecsReplicationProfileUpdateRate

USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsReplicationProfileAlwaysRelevant : public FFlecsReplicationProfileParamsBase
{
	GENERATED_BODY()
	
public:
	virtual void ApplyToEntity(const FFlecsEntityHandle& InEntity) const override;
	
}; // struct FFlecsReplicationProfileAlwaysRelevant
