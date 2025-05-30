﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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

	/* Applies to the Prefab Entity */
	UFUNCTION(BlueprintNativeEvent, Category = "Flecs Component Collection")
	void ApplyCollectionToEntity(FFlecsEntityHandle& Entity);

	void ApplyCollection_Internal(FFlecsEntityHandle Entity, UFlecsWorld* InFlecsWorld);

	UFUNCTION(BlueprintCallable, Category = "Flecs Component Collection")
	UFlecsWorld* GetFlecsWorld() const;
	
protected:

	UPROPERTY()
	TWeakObjectPtr<UFlecsWorld> FlecsWorld;

}; // class UFlecsComponentCollectionObject
