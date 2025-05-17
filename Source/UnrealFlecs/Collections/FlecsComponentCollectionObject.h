// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FlecsCollectionItemBuilder.h"
#include "Entities/FlecsEntityHandle.h"
#include "FlecsComponentCollectionObject.generated.h"

class UFlecsWorld;

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew)
class UNREALFLECS_API UFlecsComponentCollectionObject : public UObject
{
	GENERATED_BODY()

public:
	UFlecsComponentCollectionObject();
	UFlecsComponentCollectionObject(const FObjectInitializer& ObjectInitializer);

	FFlecsCollectionItemBuilder ObtainCollectionBuilder()
	{
		GetBuilder().SetNewEntity(bCreateSubEntity);
		Apply();
		return CollectionBuilder;
	}

	virtual void Apply();

	UFUNCTION(BlueprintImplementableEvent, Category = "Flecs|Component Collection", meta = (DisplayName = "On Apply"))
	void BP_OnApply();

	/**
	 * if Set to True, the collection will create a sub entity for all of
	 * its modifications rather than modifying the entity it was applied to directly
	 **/
	UPROPERTY()
	bool bCreateSubEntity = false;

	NO_DISCARD FORCEINLINE FFlecsCollectionItemBuilder& GetBuilder()
	{
		return CollectionBuilder;
	}

	NO_DISCARD FORCEINLINE const FFlecsCollectionItemBuilder& GetBuilder() const
	{
		return CollectionBuilder;
	}

	UPROPERTY()
	FFlecsCollectionItemBuilder CollectionBuilder;

}; // class UFlecsComponentCollectionObject
