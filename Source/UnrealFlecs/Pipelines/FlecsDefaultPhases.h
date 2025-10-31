// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"

#include "Entities/FlecsId.h"

#include "FlecsDefaultPhases.generated.h"

UENUM(BlueprintType)
enum class EFlecsDefaultPipelinePhase : uint8
{
	OnLoad,
	PostLoad,
	PreUpdate,
	OnUpdate,
	OnValidate,
	PostUpdate,
	PreStore,
	OnStore,
	OnStart UMETA(Hidden),
	
}; // enum class EFlecsDefaultPipelinePhase

NO_DISCARD constexpr FFlecsId ConvertDefaultPipelinePhaseToFlecsId(const EFlecsDefaultPipelinePhase InPhase)
{
	switch (InPhase)
	{
	case EFlecsDefaultPipelinePhase::OnLoad:
		return flecs::OnLoad;
	case EFlecsDefaultPipelinePhase::PostLoad:
		return flecs::PostLoad;
	case EFlecsDefaultPipelinePhase::PreUpdate:
		return flecs::PreUpdate;
	case EFlecsDefaultPipelinePhase::OnUpdate:
		return flecs::OnUpdate;
	case EFlecsDefaultPipelinePhase::OnValidate:
		return flecs::OnValidate;
	case EFlecsDefaultPipelinePhase::PostUpdate:
		return flecs::PostUpdate;
	case EFlecsDefaultPipelinePhase::PreStore:
		return flecs::PreStore;
	case EFlecsDefaultPipelinePhase::OnStore:
		return flecs::OnStore;
	case EFlecsDefaultPipelinePhase::OnStart:
		return flecs::OnStart;
	default: UNLIKELY_ATTRIBUTE
		checkNoEntry();
		return FFlecsId();
	}
}
