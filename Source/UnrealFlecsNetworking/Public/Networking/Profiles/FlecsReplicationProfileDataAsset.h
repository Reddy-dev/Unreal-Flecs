// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once


#include "Engine/DataAsset.h"

#include "Networking/Profiles/FlecsReplicationProfile.h"

#include "FlecsReplicationProfileDataAsset.generated.h"

/** Authoring source used to create a Flecs replication profile prefab. */
UCLASS(BlueprintType, Blueprintable)
class UNREALFLECSNETWORKING_API UFlecsReplicationProfileDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** Optional runtime prefab name. The asset name is used when this is unset. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Replication")
	FName ProfileName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Replication")
	FFlecsReplicationProfileDefinition Definition;

}; // class UFlecsReplicationProfileDataAsset
