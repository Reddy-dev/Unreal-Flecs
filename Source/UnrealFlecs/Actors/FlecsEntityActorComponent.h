﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Entities/FlecsEntityHandle.h"
#include "Entities/FlecsEntityRecord.h"
#include "FlecsEntityActorComponent.generated.h"

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Flecs), meta=(BlueprintSpawnableComponent,
	DisplayName = "Flecs Entity Actor Component"))
class UNREALFLECS_API UFlecsEntityActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFlecsEntityActorComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void OnRegister() override;
	virtual void OnUnregister() override;

	virtual void InitializeEntity();

	UFUNCTION(BlueprintCallable, Category = "Flecs | Entity")
	FORCEINLINE FFlecsEntityHandle GetEntityHandle() const
	{
		return EntityHandle;
	}

	UPROPERTY(EditAnywhere, Category = "Flecs | Entity")
	FFlecsEntityHandle EntityHandle;

	UPROPERTY(EditAnywhere, Category = "Flecs | Entity")
	FFlecsEntityRecord EntityRecord;

	UPROPERTY(EditAnywhere, Category = "Flecs | Entity")
	FString WorldName = TEXT("DefaultFlecsWorld");

	#if WITH_EDITORONLY_DATA

	virtual bool CanEditChange(const FProperty* InProperty) const override;

	#endif // WITH_EDITORONLY_DATA

	UFUNCTION()
	void OnWorldCreated(const FString& InWorldName, UFlecsWorld* InWorld);
	
}; // class UFlecsEntityActorComponent
