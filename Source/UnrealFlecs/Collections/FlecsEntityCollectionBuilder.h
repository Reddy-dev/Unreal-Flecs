// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FlecsCollectionTrait.h"
#include "FlecsComponentCollection.h"
#include "UObject/Object.h"
#include "FlecsEntityCollectionBuilder.generated.h"

class UFlecsCollectionTrait;

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsEntityCollectionBuilder
{
	GENERATED_BODY()

public:
	FORCEINLINE FFlecsEntityCollectionBuilder()
	{
	}

	FORCEINLINE void Build()
	{
		CollectionBuilder.Build(CollectionTraits);
	}

	NO_DISCARD FORCEINLINE bool ValidateData() const
	{
		for (const UFlecsCollectionTrait* Trait : CollectionTraits)
		{
			if UNLIKELY_IF(!IsValid(Trait))
			{
				return false;
			}
		}
		
		return CollectionBuilder.ValidateData();
	}

	template <Solid::TStaticClassConcept T>
	requires(Solid::TInheritsFromConcept<T, UFlecsCollectionTrait>)
	FORCEINLINE void AddTrait(TSolidNonNullPtr<T> InTrait)
	{
		CollectionTraits.Add(InTrait);
	}

	FORCEINLINE void ClearTraits()
	{
		CollectionTraits.Empty();
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Flecs|Collection")
	TArray<TObjectPtr<UFlecsCollectionTrait>> CollectionTraits;
	
	UPROPERTY()
	FFlecsComponentCollection CollectionBuilder;
	
}; // struct FFlecsEntityCollectionBuilder