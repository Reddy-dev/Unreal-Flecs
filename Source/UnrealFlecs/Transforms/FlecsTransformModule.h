﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Modules/FlecsModuleObject.h"
#include "FlecsTransformModule.generated.h"

UCLASS(BlueprintType, DisplayName = "Flecs Transform Module")
class UNREALFLECS_API UFlecsTransformModule final : public UFlecsModuleObject
{
	GENERATED_BODY()

public:
	UFlecsTransformModule();

	FORCEINLINE virtual FString GetModuleName_Implementation() const override
	{
		return TEXT("Flecs Transform Module");
	}

	virtual void InitializeModule(UFlecsWorld* InWorld, const FFlecsEntityHandle& InModuleEntity) override;
	virtual void DeinitializeModule(UFlecsWorld* InWorld) override;


}; // class UFlecsTransformModule