// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Entities/FlecsEntityRecord.h"
#include "FlecsCollectionTypes.h"

#include "FlecsCollectionEntityRecordFragment.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsCollectionsEntityRecordFragment : public FFlecsEntityRecordFragment
{
	GENERATED_BODY()

public:
	FORCEINLINE FFlecsCollectionsEntityRecordFragment() = default;

	FORCEINLINE FFlecsCollectionsEntityRecordFragment(const TArray<FFlecsCollectionInstancedReference>& InCollectionInstancedReference)
		: CollectionInstancedReferences(InCollectionInstancedReference)
	{
	}
	
	UPROPERTY(EditAnywhere, Category="Flecs|Entity")
	TArray<FFlecsCollectionInstancedReference> CollectionInstancedReferences;

	virtual void PostApplyRecordToEntity(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld,
		const FFlecsEntityHandle& InEntityHandle) const override;
	
	struct FBuilder;
	
}; // struct FFlecsCollectionEntityRecordFragment

struct FFlecsCollectionsEntityRecordFragment::FBuilder 
	: public FFlecsEntityRecord::FFragmentBuilderType<FFlecsCollectionsEntityRecordFragment>
{
	using Super = FFlecsEntityRecord::FFragmentBuilderType<FFlecsCollectionsEntityRecordFragment>;
	using Super::Super;
	
public:
	
	FORCEINLINE FBuilder& ReferenceCollection(const FFlecsCollectionInstancedReference& InCollectionReference)
	{
		GetSelf().CollectionInstancedReferences.Add(InCollectionReference);
		return *this;
	}
	
	FORCEINLINE FBuilder& ReferenceCollection(const FFlecsCollectionReference& InCollectionReference,
		const FInstancedStruct& InParameters = FInstancedStruct())
	{
		GetSelf().CollectionInstancedReferences.Add(
			FFlecsCollectionInstancedReference{ InCollectionReference, InParameters });
		return *this;
	}
	
	template <Solid::TScriptStructConcept TCollectionParams>
	FORCEINLINE FBuilder& ReferenceCollection(const FFlecsCollectionReference& InCollectionReference,
		const TCollectionParams& InParameters)
	{
		return ReferenceCollection(InCollectionReference, FInstancedStruct::Make<TCollectionParams>(InParameters));
	}
	
}; // struct FFlecsCollectionsEntityRecordFragment::FBuilder
