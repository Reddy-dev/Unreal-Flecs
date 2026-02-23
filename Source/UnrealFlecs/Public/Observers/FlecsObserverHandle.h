// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "Entities/FlecsEntityHandle.h"
#include "FlecsObserverDefinition.h"

#include "FlecsObserverHandle.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsObserverHandle : public FFlecsEntityHandle
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsObserverHandle() = default;
	
	FFlecsObserverHandle(const TSolidNotNull<const UFlecsWorld*> InWorld, 
		const FFlecsObserverDefinition& InObserverBuilder, const FString& InObserverName);
	
private:
	flecs::observer Observer;
	
}; // struct FFlecsObserverHandle