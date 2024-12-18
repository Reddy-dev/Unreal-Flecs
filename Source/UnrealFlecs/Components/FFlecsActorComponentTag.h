﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Properties/FlecsComponentProperties.h"
#include "FFlecsActorComponentTag.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsActorComponentTag
{
	GENERATED_BODY()
}; // struct FFlecsActorComponentTag

REGISTER_COMPONENT_TAG_PROPERTIES(FFlecsActorComponentTag, flecs::PairIsTag);
