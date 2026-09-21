// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "Entities/FlecsEntityHandle.h"
#include "FlecsObserverDefinition.h"

#include "FlecsObserverHandle.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsObserverHandle final : public FFlecsEntityHandle
{
	GENERATED_BODY()
	
public:
	using FFlecsEntityHandle::FFlecsEntityHandle;
	
	FFlecsObserverHandle(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, 
		const FFlecsObserverDefinition& InObserverBuilder, const FString& InObserverName);
	
	FFlecsObserverHandle(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld,
		const FFlecsObserverDefinition& InObserverBuilder, const FFlecsId InExistingEntity);

	FORCEINLINE FFlecsObserverHandle(const flecs::observer& InObserver)
		: FFlecsEntityHandle(InObserver.world(), InObserver.id())
	{
	}
	
	NO_DISCARD FORCEINLINE flecs::observer GetObserver() const
	{
		return flecs::observer(GetNativeFlecsWorld(), GetFlecsId());
	}
	
}; // struct FFlecsObserverHandle
