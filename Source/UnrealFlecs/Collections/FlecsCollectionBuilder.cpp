// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsCollectionBuilder.h"
#include "Entities/FlecsEntityHandle.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsCollectionBuilder)

void FFlecsCollectionBuilder::ApplyToEntity(const FFlecsEntityHandle& InEntityHandle, TSolidNonNullPtr<UFlecsWorld> InWorld) const
{
	solid_check(IsValid(InWorld));
	solid_check(InEntityHandle.IsValid());
	
	for (const FString& ComponentName : ComponentRequirements)
	{
		const FFlecsId ComponentId = ecs_lookup_symbol(InWorld->World,
			StringCast<char>(*ComponentName).Get(), false, false);

		if UNLIKELY_IF(!InEntityHandle.Has(ComponentId))
		{
			// @TODO: Check if we want to return or crash.
			UN_LOGF(LogFlecsWorld, Error,
				"Component %s is required for collection", *ComponentName);
			return;
		}
	}

	for (const FString& ComponentName : ComponentExclusions)
	{
		const FFlecsId ComponentId = ecs_lookup_symbol(InWorld->World,
			StringCast<char>(*ComponentName).Get(), false, false);

		if UNLIKELY_IF(InEntityHandle.Has(ComponentId))
		{
			// @TODO: Check if we want to return or crash.
			UN_LOGF(LogFlecsWorld, Error,
				"Component %s is excluded from collection", *ComponentName);
			return;
		}
	}

	for (const FFlecsCollectionItem& Item : CollectionItems)
	{
		const bool bIsPair = Item.Second.IsEmpty();
		
		const FFlecsId FirstComponentId = ecs_lookup_symbol(InWorld->World,
			StringCast<char>(*Item.First).Get(), false, false);
		
		FFlecsId SecondComponentId;

		if (bIsPair)
		{
			SecondComponentId = ecs_lookup_symbol(InWorld->World,
				StringCast<char>(*Item.Second).Get(), false, false);
		}

		if (Item.ComponentData.IsEmpty())
		{
			if (bIsPair)
			{
				InEntityHandle.AddPair(FirstComponentId, SecondComponentId);
			}
			else
			{
				InEntityHandle.Add(FirstComponentId);
			}
		}
		else
		{
			if (bIsPair)
			{
				FFlecsEntityHandle FirstEntityHandle = InWorld->GetEntity(FirstComponentId);
				FFlecsEntityHandle SecondEntityHandle = InWorld->GetEntity(SecondComponentId);
				
				// ReSharper disable once CppTooWideScope
				const bool bIsFirstData = !FirstEntityHandle.IsTag();

				if (bIsFirstData)
				{
					InEntityHandle.SetPairFirst(FirstEntityHandle,
						Item.ComponentData.GetData(), SecondComponentId);
				}
				else if (SecondEntityHandle.IsTag())
				{
					InEntityHandle.SetPairSecond(FirstComponentId, SecondComponentId,
						Item.ComponentData.GetData());
				}
				else UNLIKELY_ATTRIBUTE
				{
					ensureMsgf(false,
						TEXT("Both components are tags, cannot set data"));
				}
			}
			else
			{
				InEntityHandle.Set(FirstComponentId,
					Item.ComponentData.GetData());
			}
		}
	}
}

bool FFlecsCollectionBuilder::ValidateData(FFlecsComponentCollection& InComponentCollection) const
{
	for (const FFlecsCollectionItem& Item : CollectionItems)
	{
		const bool bIsPair = Item.Second.IsEmpty();
	}

	return true;
}
