// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Queries/Generator/FlecsQueryGeneratorInput.h"
#include "FlecsPhasesType.h"

#include "FlecsSystemPhaseInput.generated.h"

UENUM(BlueprintType)
enum class EFlecsSystemPhaseInputType : uint8
{
	None = 0,
	FlecsPhase,
	Type,
}; // enum class EFlecsSystemPhaseInputType

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsSystemPhaseInput
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsSystemPhaseInput() = default;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EFlecsSystemPhaseInputType Type = EFlecsSystemPhaseInputType::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Type == EFlecsSystemPhaseInputType::FlecsPhase", EditConditionHides))
	EFlecsPhaseType FlecsPhase = EFlecsPhaseType::PreUpdate;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (EditCondition = "Type == EFlecsSystemPhaseInputType::Type", EditConditionHides, AllowStringInput = false))
	FFlecsQueryGeneratorInput PhaseInput;
	
	void ApplyToSystemDefinition(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InFlecsWorld, flecs::system_builder<>& InSystemBuilder) const;
	
}; // struct FFlecsSystemPhaseInput

