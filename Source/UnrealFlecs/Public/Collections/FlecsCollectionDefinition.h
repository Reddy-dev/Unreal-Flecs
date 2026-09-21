// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "EntityRecords/FlecsEntityRecord.h"
#include "FlecsCollectionTypes.h"

#include "FlecsCollectionDefinition.generated.h"

USTRUCT(BlueprintType)
/**
 * @brief Collection references associated with one sub-entity slot.
 *
 * The map key in FFlecsCollectionDefinition identifies the sub-entity record
 * index; this value supplies its optional name and nested Collection references.
 */
struct UNREALFLECS_API FFlecsSubEntityCollectionReferences
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Flecs")
	/** Optional name assigned to the instantiated sub-entity. */
	FString SubEntityName;

	UPROPERTY(EditAnywhere, Category = "Flecs")
	/** Collections to apply to the instantiated sub-entity. */
	TArray<FFlecsCollectionInstancedReference> CollectionReferences;
	
}; // struct FFlecsSubEntityCollectionReferences

USTRUCT(BlueprintType)
/**
 * @brief Unreal-owned definition used to register a Collection prefab.
 *
 * A definition combines a root FFlecsEntityRecord with referenced Collections
 * and per-sub-entity references. The Collection subsystem resolves this data
 * into a Flecs prefab entity.
 */
struct UNREALFLECS_API FFlecsCollectionDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Flecs")
	/** Optional display/name key for the Collection definition. */
	FString Name;
	
	UPROPERTY(EditAnywhere, Category = "Flecs")
	/** Components and child entities applied to the Collection prefab. */
	FFlecsEntityRecord Record;

	UPROPERTY(EditAnywhere, Category = "Flecs")
	/** Collections composed into the root prefab. */
	TArray<FFlecsCollectionInstancedReference> Collections;

	// @TODO: make this an array
	UPROPERTY(EditAnywhere, Category = "Flecs")
	/** Nested Collection references keyed by sub-entity record index. */
	TMap<int32, FFlecsSubEntityCollectionReferences> SubEntityCollections;
	
}; // struct FFlecsCollectionDefinition

USTRUCT(BlueprintType)
/**
 * @brief Stores the source definition on a registered Collection prefab.
 *
 * This component is retained on the prefab for inspection and parameter
 * expansion; it is not inherited when an entity instantiates the Collection.
 */
struct UNREALFLECS_API FFlecsCollectionDefinitionComponent
{
	GENERATED_BODY()
	
	static constexpr flecs::on_instantiate OnInstantiate = flecs::on_instantiate::dont_inherit;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flecs")
	/** Definition used to create the owning Collection prefab. */
	FFlecsCollectionDefinition Definition;
	
}; // struct FFlecsCollectionDefinitionComponent

FLECS_COMPONENT_TRAITS(FFlecsCollectionDefinitionComponent)
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::DontInherit;
}; // struct TFlecsComponentTraits<FFlecsCollectionDefinitionComponent>
