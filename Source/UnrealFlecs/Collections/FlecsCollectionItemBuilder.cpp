// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsCollectionItemBuilder.h"
#include "Entities/FlecsEntityHandle.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsCollectionItemBuilder)

void FFlecsCollectionItemBuilder::ApplyToEntity(const FFlecsEntityHandle& InEntityHandle, UFlecsWorld* InWorld) const
{
	solid_check(IsValid(InWorld));
	solid_check(InEntityHandle.IsValid());

	FFlecsEntityHandle UsedEntityHandle;
	
	if (IsNewEntity())
	{
		UsedEntityHandle = InWorld->CreateEntity();
		UsedEntityHandle.SetParent(InEntityHandle);
	}
	else
	{
		UsedEntityHandle = InEntityHandle;
	}

	for (const FString& ComponentName : ComponentRequirements)
	{
		const FFlecsId ComponentId = ecs_lookup_symbol(InWorld->World,
			StringCast<char>(*ComponentName).Get(), false, false);

		if UNLIKELY_IF(!UsedEntityHandle.Has(ComponentId))
		{
			// @TODO: Check if we want to return or error.
			return;
		}
	}

	for (const FString& ComponentName : ComponentExclusions)
	{
		const FFlecsId ComponentId = ecs_lookup_symbol(InWorld->World,
			StringCast<char>(*ComponentName).Get(), false, false);

		if UNLIKELY_IF(UsedEntityHandle.Has(ComponentId))
		{
			return;
		}
	}

	for (const FFlecsCollectionItem& Item : CollectionItems)
	{
		const FFlecsId ComponentId = ecs_lookup_symbol(InWorld->World,
			StringCast<char>(*Item.ComponentName).Get(), false, false);

		if (Item.ComponentData.IsEmpty())
		{
			UsedEntityHandle.Add(ComponentId);
		}
		else
		{
			const void* Data = Item.ComponentData.GetData();
			UsedEntityHandle.Set(ComponentId, Data);
		}
	}
}
