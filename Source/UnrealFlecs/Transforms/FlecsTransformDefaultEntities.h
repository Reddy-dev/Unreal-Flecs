﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "flecs.h"
#include "Entities/FlecsDefaultEntityEngineSubsystem.h"
#include "FlecsTransformDefaultEntities.generated.h"

DEFINE_DEFAULT_ENTITY_OPTION(FlecsLocalTrait);
DEFINE_DEFAULT_ENTITY_OPTION(FlecsGlobalTrait);

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsRelativeTrait
{
	GENERATED_BODY()
	
	// How far up the hierarchy to look for the relative entity, 0 is the Parent, 1 is the Parent's Parent, etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Transform")
	int32 RelativeToIndex = 0;
}; // struct FFlecsRelativeComponent

USTRUCT()
struct UNREALFLECS_API FFlecsRelativeCacheTrait
{
	GENERATED_BODY()

	UPROPERTY()
	FFlecsEntityHandle RelativeEntity;
}; // struct FFlecsRelativeCacheTrait