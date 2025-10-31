// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsTransformTypes.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsVectorComponent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Value;
	
}; // struct FFlecsVectorComponent

template <>
struct TIsPODType<FFlecsVectorComponent>
{
	enum { Value = true };
}; // struct TIsPODType<FFlecsLocationComponent>

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsQuatComponent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FQuat Value;
	
}; // struct FFlecsQuatComponent

template <>
struct TIsPODType<FFlecsQuatComponent>
{
	enum { Value = true };
};


