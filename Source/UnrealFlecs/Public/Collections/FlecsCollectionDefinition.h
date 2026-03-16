// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "EntityRecords/FlecsEntityRecord.h"
#include "FlecsCollectionTypes.h"

#include "FlecsCollectionDefinition.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsSubEntityCollectionReferences
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Flecs")
	FString SubEntityName;

	UPROPERTY(EditAnywhere, Category = "Flecs")
	TArray<FFlecsCollectionInstancedReference> CollectionReferences;
	
}; // struct FFlecsSubEntityCollectionReferences

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsCollectionDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Flecs")
	FString Name;
	
	UPROPERTY(EditAnywhere, Category = "Flecs")
	FFlecsEntityRecord Record;

	UPROPERTY(EditAnywhere, Category = "Flecs")
	TArray<FFlecsCollectionInstancedReference> Collections;

	// @TODO: make this an array
	UPROPERTY(EditAnywhere, Category = "Flecs")
	TMap<int32, FFlecsSubEntityCollectionReferences> SubEntityCollections;
	
}; // struct FFlecsCollectionDefinition

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsCollectionDefinitionComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flecs")
	FFlecsCollectionDefinition Definition;
	
}; // struct FFlecsCollectionDefinitionComponent

template<>
struct TFlecsComponentTraits<FFlecsCollectionDefinitionComponent> : public TFlecsComponentTraitsBase<FFlecsCollectionDefinitionComponent>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::DontInherit;
}; // struct TFlecsComponentTraits<FFlecsCollectionDefinitionComponent>
