﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"

#include "Entities/FlecsEntityHandle.h"
#include "Interfaces/FlecsEntityInterface.h"

#include "FlecsPrefabObject.generated.h"

class UFlecsWorld;

UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, ClassGroup = (Flecs))
class UNREALFLECS_API UFlecsPrefabObject : public UObject, public IFlecsEntityInterface
{
	GENERATED_BODY()

public:
	void CreatePrefab(TSolidNotNull<UFlecsWorld*> InWorld);
	
	virtual void OnPrefabCreated(const FFlecsEntityHandle& InPrefab) {}

	UFUNCTION(BlueprintImplementableEvent, Category = "Flecs", meta = (DisplayName = "OnPrefabCreated"))
	void BP_OnPrefabCreated(const FFlecsEntityHandle& InPrefab);

	UPROPERTY(EditAnywhere, Category = "Flecs")
	FString PrefabName;

	virtual FFlecsEntityHandle GetEntityHandle() const override final;

	UPROPERTY()
	FFlecsEntityHandle PrefabEntityHandle;

	UFUNCTION(BlueprintCallable, Category = "Flecs")
	FORCEINLINE FFlecsEntityHandle GetPrefabHandle() const
	{
		return PrefabEntityHandle;
	}

	UFUNCTION(BlueprintCallable, Category = "Flecs")
	FORCEINLINE UFlecsWorld* GetFlecsWorld() const
	{
		return FlecsWorld.Get();
	}

protected:
	UPROPERTY()
	TWeakObjectPtr<UFlecsWorld> FlecsWorld;
	
}; // class UFlecsPrefabObject
