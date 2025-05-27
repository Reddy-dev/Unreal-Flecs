// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FlecsCollectionBuilder.h"
#include "FlecsCollectionTrait.h"
#include "FlecsComponentCollection.generated.h"

USTRUCT()
struct UNREALFLECS_API FFlecsComponentCollection
{
	GENERATED_BODY()

public:
	FORCEINLINE FFlecsComponentCollection()
	{
		CollectionItems.SetNum(1);
	}

	FORCEINLINE void Build(TArrayView<TObjectPtr<UFlecsCollectionTrait>> InTraits)
	{
		for (const TObjectPtr<UFlecsCollectionTrait>& Trait : InTraits)
		{
			switch (Trait->GetTraitType())
			{
				case EFlecsCollectionTraitType::None:
					{
						Trait->Build(CollectionItems[0]);
					}
					break;
				case EFlecsCollectionTraitType::NewEntity:
					{
						FFlecsCollectionBuilder& Builder = CollectionItems.Add_GetRef(FFlecsCollectionBuilder());
						Trait->Build(Builder);
					}
					break;
				case EFlecsCollectionTraitType::SlotEntity:
					{
						FFlecsCollectionBuilder& Builder = CollectionItems.Add_GetRef(FFlecsCollectionBuilder(true));
						Trait->Build(Builder);
					}
					break;
			}
		}
	}
	
	NO_DISCARD FORCEINLINE bool ValidateData() const
	{
		for (const FFlecsCollectionBuilder& Item : CollectionItems)
		{
			if UNLIKELY_IF(!Item.ValidateData(const_cast<FFlecsComponentCollection&>(*this)))
			{
				return false;
			}
		}

		return true;
	}

	NO_DISCARD FORCEINLINE TArrayView<const FFlecsCollectionBuilder> GetCollectionItems() const
	{
		return CollectionItems;
	}

private:
	// First is the Base Entity, any indices afterward are child entities
	TArray<FFlecsCollectionBuilder, TInlineAllocator<1>> CollectionItems;
}; // struct FFlecsComponentCollection

