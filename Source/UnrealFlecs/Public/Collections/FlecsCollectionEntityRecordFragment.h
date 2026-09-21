// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "EntityRecords/FlecsEntityRecord.h"
#include "FlecsCollectionTypes.h"

#include "FlecsCollectionEntityRecordFragment.generated.h"

USTRUCT(BlueprintType)
/**
 * @brief Entity-record fragment that applies Collections after record creation.
 *
 * Each stored reference is added to the instantiated entity by the Collection
 * subsystem during the fragment's post-application phase.
 */
struct UNREALFLECS_API FFlecsCollectionsEntityRecordFragment : public FFlecsEntityRecordFragment
{
	GENERATED_BODY()

public:
	/** Creates an empty Collection-reference fragment. */
	FORCEINLINE FFlecsCollectionsEntityRecordFragment() = default;

	/**
	 * @brief Creates a fragment from Collection references.
	 * @param InCollectionInstancedReference References applied after creation.
	 */
	FORCEINLINE FFlecsCollectionsEntityRecordFragment(const TArray<FFlecsCollectionInstancedReference>& InCollectionInstancedReference)
		: CollectionInstancedReferences(InCollectionInstancedReference)
	{
	}
	
	UPROPERTY(EditAnywhere, Category="Flecs|Entity")
	/** Collection references applied to the instantiated entity. */
	TArray<FFlecsCollectionInstancedReference> CollectionInstancedReferences;

	/**
	 * @brief Applies each stored Collection reference after record creation.
	 * @param InFlecsWorld World used to resolve the Collection subsystem.
	 * @param InEntityHandle Entity receiving the references.
	 */
	virtual void PostApplyRecordToEntity(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InFlecsWorld,
		const FFlecsEntityHandle& InEntityHandle) const override;
	
	/** Fluent builder for this fragment's Collection references. */
	struct FBuilder;
	
}; // struct FFlecsCollectionEntityRecordFragment

struct FFlecsCollectionsEntityRecordFragment::FBuilder 
	: public FFlecsEntityRecord::TFragmentBuilderType<FFlecsCollectionsEntityRecordFragment>
{
	using Super = FFlecsEntityRecord::TFragmentBuilderType<FFlecsCollectionsEntityRecordFragment>;
	using Super::Super;
	
public:
	
	/**
	 * @brief Adds a complete instanced Collection reference.
	 * @param InCollectionReference Reference and parameters to store.
	 * @return This fragment builder for chaining.
	 */
	FORCEINLINE FBuilder& ReferenceCollection(const FFlecsCollectionInstancedReference& InCollectionReference)
	{
		GetSelf().CollectionInstancedReferences.Add(InCollectionReference);
		return *this;
	}
	
	/**
	 * @brief Adds a Collection reference and optional parameters.
	 * @param InCollectionReference Collection reference to store.
	 * @param InParameters Parameters supplied when the fragment is applied.
	 * @return This fragment builder for chaining.
	 */
	FORCEINLINE FBuilder& ReferenceCollection(const FFlecsCollectionReference& InCollectionReference,
		const FInstancedStruct& InParameters = FInstancedStruct())
	{
		GetSelf().CollectionInstancedReferences.Add(
			FFlecsCollectionInstancedReference{ InCollectionReference, InParameters });
		return *this;
	}
	
	template <Solid::TScriptStructConcept TCollectionParams>
	/**
	 * @brief Adds a Collection reference with typed parameters.
	 * @tparam TCollectionParams Script-struct type of the parameters.
	 * @param InCollectionReference Collection reference to store.
	 * @param InParameters Parameters supplied when the fragment is applied.
	 * @return This fragment builder for chaining.
	 */
	FORCEINLINE FBuilder& ReferenceCollection(const FFlecsCollectionReference& InCollectionReference,
		const TCollectionParams& InParameters)
	{
		return ReferenceCollection(InCollectionReference, FInstancedStruct::Make<TCollectionParams>(InParameters));
	}
	
/**
 * @brief Builder for FFlecsCollectionsEntityRecordFragment.
 */
}; // struct FFlecsCollectionsEntityRecordFragment::FBuilder
