// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Properties/FlecsComponentProperties.h"
#include "FlecsTranslationSettingTraits.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsTranslationPropertyTrait
{
	GENERATED_BODY()
}; // struct FFlecsTranslationPropertyTrait

REGISTER_FLECS_COMPONENT(FFlecsTranslationPropertyTrait, [](flecs::world InWorld, flecs::untyped_component InComponent)
{
	InComponent.add(flecs::PairIsTag);
})

UENUM(BlueprintType)
enum class EFlecsTranslationType : uint8
{
	None = 0,
	InitializationOnly = 1,
	FlecsToUnreal = 1 << 1,
	UnrealToFlecs = 1 << 2,
	ToBoth = FlecsToUnreal | UnrealToFlecs,
}; // enum class EFlecsTranslationType

ENUM_CLASS_FLAGS(EFlecsTranslationType);

/*USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsInitializationTrait
{
	GENERATED_BODY()
}; // struct FFlecsInitializationTrait

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsToUnreal
{
	GENERATED_BODY()
}; // struct FFlecsToUnreal

USTRUCT(BlueprintType)
struct UNREALFLECS_API FUnrealToFlecs
{
	GENERATED_BODY()
}; // struct FUnrealToFlecs

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsToBoth
{
	GENERATED_BODY()
}; // struct FFlecsToBoth*/