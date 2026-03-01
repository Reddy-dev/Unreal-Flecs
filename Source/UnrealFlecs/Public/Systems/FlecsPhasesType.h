// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"

#include "Entities/FlecsId.h"

#include "FlecsPhasesType.generated.h"

UENUM(BlueprintType)
enum class EFlecsPhaseType : uint8
{
	OnStart = 0,
	PreFrame,
	OnLoad,
	PostLoad,
	PreUpdate,
	OnUpdate,
	OnValidate,
	PostUpdate,
	PreStore,
	OnStore,
	PostFrame,
	Count UMETA(Hidden)
}; // enum class EFlecsPhaseType
ENUM_RANGE_BY_COUNT(EFlecsPhaseType, EFlecsPhaseType::Count);

NO_DISCARD FORCEINLINE FFlecsId ConvertPhaseTypeToId(const EFlecsPhaseType InPhaseType)
{
	switch (InPhaseType)
	{
		case EFlecsPhaseType::OnStart:
			return flecs::OnStart;
			break;
		case EFlecsPhaseType::PreFrame:
			return flecs::PreFrame;
			break;
		case EFlecsPhaseType::OnLoad:
			return flecs::OnLoad;
			break;
		case EFlecsPhaseType::PostLoad:
			return flecs::PostLoad;
			break;
		case EFlecsPhaseType::PreUpdate:
			return flecs::PreUpdate;
			break;
		case EFlecsPhaseType::OnUpdate:
			return flecs::OnUpdate;
			break;
		case EFlecsPhaseType::OnValidate:
			return flecs::OnValidate;
			break;
		case EFlecsPhaseType::PostUpdate:
			return flecs::PostUpdate;
			break;
		case EFlecsPhaseType::PreStore:
			return flecs::PreStore;
			break;
		case EFlecsPhaseType::OnStore:
			return flecs::OnStore;
			break;
		case EFlecsPhaseType::PostFrame:
			return flecs::PostFrame;
			break;
		case EFlecsPhaseType::Count:
			UNREACHABLE
			break;
	}
	
	UNREACHABLE
	return FFlecsId::Null();
}
