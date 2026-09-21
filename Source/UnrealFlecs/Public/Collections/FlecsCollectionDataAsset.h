// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "Versioning/SolidVersioningTypes.h"

#include "EntityRecords/FlecsEntityRecord.h"
#include "FlecsCollectionTypes.h"
#include "FlecsCollectionDefinition.h"

#include "FlecsCollectionDataAsset.generated.h"

START_SOLID_ASSET_VERSION(UFlecsCollectionDataAsset)

END_SOLID_ASSET_VERSION() // UFlecsCollectionDataAsset

UCLASS(BlueprintType, Blueprintable)
/**
 * @brief Primary data asset that describes a reusable Collection.
 *
 * The asset stores a root entity record plus composed Collections and
 * per-sub-entity references. Register it with
 * UFlecsCollectionWorldSubsystem::RegisterCollectionAsset() to create the
 * corresponding Flecs prefab in a world.
 */
class UNREALFLECS_API UFlecsCollectionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Creates an empty Collection data asset. */
	UFlecsCollectionDataAsset();

	/**
	 * @brief Returns the primary asset id used by the Asset Manager.
	 * @return The id for this Collection data asset.
	 */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override final;

	/** Collections composed into the root prefab. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TArray<FFlecsCollectionInstancedReference> Collections;

	/** Components and child entities in the root Collection record. */
	UPROPERTY(EditAnywhere, Category = "Collections")
	FFlecsEntityRecord Record;

	/** Nested Collection references keyed by sub-entity record index. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TMap<int32, FFlecsSubEntityCollectionReferences> SubEntityCollections;

	/**
	 * @brief Copies this asset into a registration-ready Collection definition.
	 * @return The Collection definition represented by this asset.
	 */
	NO_DISCARD FFlecsCollectionDefinition MakeCollectionDefinition() const;

#if WITH_EDITOR
	/**
	 * @brief Validates the asset using Unreal's data-validation pipeline.
	 * @param Context Validation context that receives any errors.
	 * @return Combined validation result from the base asset and Collection data.
	 */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
	
	/**
	 * @brief Validates Collection references stored by this asset.
	 * @param Context Validation context that receives any errors.
	 * @return Validation result for the Collection-specific data.
	 */
	NO_DISCARD EDataValidationResult ValidateCollections(FDataValidationContext& Context) const;
#endif // WITH_EDITOR

}; // class UFlecsCollectionDataAsset
