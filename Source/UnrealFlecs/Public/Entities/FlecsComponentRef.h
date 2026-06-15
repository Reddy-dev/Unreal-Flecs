// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Entities/FlecsEntityView.h"
#include "Entities/FlecsId.h"

#include "FlecsComponentRef.generated.h"

USTRUCT(BlueprintType, meta = (DisableSplitPin))
struct UNREALFLECS_API FFlecsComponentRef
{
	GENERATED_BODY()

	FFlecsComponentRef() = default;

	FFlecsComponentRef(const FFlecsEntityView& InEntity, const FFlecsId InComponentId)
		: Entity(InEntity),
		  ComponentId(InComponentId)
	{
	}

	NO_DISCARD bool IsValid() const
	{
		return Entity.IsValid() &&
			ComponentId.IsValid() &&
			Entity.TryGet(ComponentId) != nullptr;
	}

	NO_DISCARD const void* TryGet() const
	{
		return Entity.IsValid() && ComponentId.IsValid()
			? Entity.TryGet(ComponentId)
			: nullptr;
	}

	NO_DISCARD void* TryGetMutable() const
	{
		return Entity.IsValid() && ComponentId.IsValid()
			? Entity.TryGetMut(ComponentId)
			: nullptr;
	}

	void Modified() const
	{
		if (Entity.IsValid() && ComponentId.IsValid())
		{
			Entity.GetEntity().modified(ComponentId);
		}
	}

	UPROPERTY()
	FFlecsEntityView Entity;

	UPROPERTY()
	FFlecsId ComponentId;

}; // struct FFlecsComponentRef
