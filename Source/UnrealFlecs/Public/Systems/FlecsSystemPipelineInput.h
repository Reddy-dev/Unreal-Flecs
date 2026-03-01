// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

#include "Queries/Generator/FlecsQueryGeneratorInput.h"

#include "FlecsSystemPipelineInput.generated.h"

struct FFlecsSystemHandle;

UENUM(BlueprintType)
enum class EFlecsSystemPipelineInputType : uint8
{
	None = 0,
	Tag,
	Type,
}; // enum class EFlecsSystemPipelineInputType

// Applied as a component to an already built system
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsSystemPipelineInput
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EFlecsSystemPipelineInputType InputType = EFlecsSystemPipelineInputType::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "InputType == EFlecsSystemPipelineInputType::Tag", EditConditionHides))
	FGameplayTag Tag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "InputType == EFlecsSystemPipelineInputType::Type", EditConditionHides, AllowStringInput = false))
	FFlecsQueryGeneratorInput TypeInput;
	
	void ApplyToSystemEntity(const TSolidNotNull<const UFlecsWorld*> InWorld, const FFlecsSystemHandle& InSystem) const;
	
}; // struct FFlecsSystemPipelineInput
