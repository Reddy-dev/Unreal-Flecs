// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Properties/FlecsComponentProperties.h"

#include "FlecsOutsideOfMainLoopTag.generated.h"


USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsOutsideOfMainLoopTag
{
	GENERATED_BODY()
}; // struct FFlecsOutsideOfMainLoopTag

FLECS_COMPONENT_TRAITS(FFlecsOutsideOfMainLoopTag)
{
	static constexpr bool UseLowId = true;
}; // struct FLECS_COMPONENT_TRAITS(FFlecsOutsideOfMainLoopTag)