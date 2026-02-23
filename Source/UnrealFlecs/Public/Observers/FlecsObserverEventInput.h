// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "StructUtils/InstancedStruct.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "Entities/FlecsId.h"
#include "Queries/Generator/FlecsQueryGeneratorInput.h"

#include "FlecsObserverEventInput.generated.h"

class UFlecsWorld;

UENUM(BlueprintType)
enum class EFlecsObserverEvent : uint8
{
	OnAdd = 0,
	OnRemove,
	OnSet,
	OnDelete,
	OnDeleteTarget,
	OnTableCreate,
	OnTableDelete,
	Count UMETA(Hidden)
}; // enum class EFlecsObserverEvent
ENUM_RANGE_BY_COUNT(EFlecsObserverEvent, EFlecsObserverEvent::Count);

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsObserverEventInput
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsObserverEventInput() = default;
	
	static NO_DISCARD FFlecsObserverEventInput Make(const FFlecsId InEventId);
	static NO_DISCARD FFlecsObserverEventInput Make(const EFlecsObserverEvent InEventType);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCustomEventType = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "!bCustomEventType", EditConditionHides))
	EFlecsObserverEvent EventType = EFlecsObserverEvent::OnAdd;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bCustomEventType", EditConditionHides, AllowPair = false))
	FFlecsQueryGeneratorInput CustomEventType;
	
	void ApplyToObserver(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld,
		flecs::observer_builder<>& InObserverBuilder) const;
	
}; // struct FFlecsObserverEventInput
