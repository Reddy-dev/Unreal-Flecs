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

USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsReplicationProfileObjectPrioritizer : public FFlecsReplicationProfileParamsBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	FName Name = NAME_None;

	NO_DISCARD bool operator==(const FFlecsReplicationProfileObjectPrioritizer& Other) const
	{
		return Name == Other.Name;
	}

	NO_DISCARD bool operator!=(const FFlecsReplicationProfileObjectPrioritizer& Other) const
	{
		return !(*this == Other);
	}

	virtual void ApplyToEntity(const FFlecsEntityHandle& InEntity) const override;

}; // struct FFlecsReplicationProfileObjectPrioritizer

USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsReplicationProfileNetFilter : public FFlecsReplicationProfileParamsBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	FName Name = NAME_None;

	NO_DISCARD bool operator==(const FFlecsReplicationProfileNetFilter& Other) const
	{
		return Name == Other.Name;
	}

	NO_DISCARD bool operator!=(const FFlecsReplicationProfileNetFilter& Other) const
	{
		return !(*this == Other);
	}

	virtual void ApplyToEntity(const FFlecsEntityHandle& InEntity) const override;

}; // struct FFlecsReplicationProfileNetFilter

USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsReplicationProfileNetShardSelector : public FFlecsReplicationProfileParamsBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	FName Name = NAME_None;

	NO_DISCARD bool operator==(const FFlecsReplicationProfileNetShardSelector& Other) const
	{
		return Name == Other.Name;
	}

	NO_DISCARD bool operator!=(const FFlecsReplicationProfileNetShardSelector& Other) const
	{
		return !(*this == Other);
	}

	virtual void ApplyToEntity(const FFlecsEntityHandle& InEntity) const override;

}; // struct FFlecsReplicationProfileNetShardSelector
