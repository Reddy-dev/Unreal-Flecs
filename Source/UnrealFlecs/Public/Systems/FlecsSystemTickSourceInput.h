// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Queries/Generator/FlecsQueryGeneratorInput.h"

#include "FlecsSystemTickSourceInput.generated.h"

class UFlecsSystemObject;

UENUM(BlueprintType)
enum class EFlecsSystemTickSourceInput : uint8
{
	None,
	Type,
	SystemClass,
}; // enum class EFlecsSystemTickSourceInput

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsSystemTickSourceInput
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsSystemTickSourceInput() = default;
	
	UPROPERTY(EditAnywhere, Category = "Flecs System Tick Source")
	EFlecsSystemTickSourceInput InputType = EFlecsSystemTickSourceInput::None;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition = "InputType == EFlecsSystemTickSourceInput::Type", EditConditionHides, AllowsStringInput = false, AllowsPairInput = false))
	FFlecsQueryGeneratorInput TypeInput;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition = "InputType == EFlecsSystemTickSourceInput::SystemClass", EditConditionHides))
	TSubclassOf<UFlecsSystemObject> SystemClassInput;
	
	void ApplyToSystemDefinition(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InFlecsWorld, flecs::system_builder<>& InSystemBuilder) const;
	
}; // struct FFlecsSystemTickSourceInput