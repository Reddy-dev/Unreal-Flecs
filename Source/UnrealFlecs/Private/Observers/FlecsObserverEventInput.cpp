// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Observers/FlecsObserverEventInput.h"

#include "Logs/FlecsCategories.h"
#include "Queries/Generator/FlecsQueryGeneratorInputType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsObserverEventInput)

FFlecsObserverEventInput FFlecsObserverEventInput::Make(const FFlecsId InEventId)
{
	solid_checkf(InEventId.IsValid(), TEXT("Event ID must be valid."));
	
	FFlecsObserverEventInput Result;
	
	if (InEventId.GetId() == flecs::OnAdd)
	{
		Result.EventType = EFlecsObserverEvent::OnAdd;
	}
	else if (InEventId.GetId() == flecs::OnRemove)
	{
		Result.EventType = EFlecsObserverEvent::OnRemove;
	}
	else if (InEventId.GetId() == flecs::OnSet)
	{
		Result.EventType = EFlecsObserverEvent::OnSet;
	}
	else if (InEventId.GetId() == flecs::OnDelete)
	{
		Result.EventType = EFlecsObserverEvent::OnDelete;
	}
	else if (InEventId.GetId() == flecs::OnDeleteTarget)
	{
		Result.EventType = EFlecsObserverEvent::OnDeleteTarget;
	}
	else if (InEventId.GetId() == flecs::OnTableCreate)
	{
		Result.EventType = EFlecsObserverEvent::OnTableCreate;
	}
	else if (InEventId.GetId() == flecs::OnTableDelete)
	{
		Result.EventType = EFlecsObserverEvent::OnTableDelete;
	}
	else
	{
		Result.bCustomEventType = true;
		Result.CustomEventType.First.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		Result.CustomEventType.First.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InEventId;
	}
	
	return Result;
}

FFlecsObserverEventInput FFlecsObserverEventInput::Make(const EFlecsObserverEvent InEventType)
{
	solid_checkf(InEventType != EFlecsObserverEvent::Count, TEXT("Event type must be a valid value other than Count."));
	
	FFlecsObserverEventInput Result;
	Result.EventType = InEventType;
	return Result;
}

void FFlecsObserverEventInput::ApplyToObserver(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InFlecsWorld, flecs::observer_builder<>& InObserverBuilder) const
{
	if (bCustomEventType)
	{
		if (CustomEventType.IsValid())
		{
			if (CustomEventType.bPair)
			{
				FFlecsTermRefAtom_Internal FirstAtom = CustomEventType.GetFirstTermRef(InFlecsWorld);
				FFlecsTermRefAtom_Internal SecondAtom = CustomEventType.GetSecondTermRef(InFlecsWorld);
				
				solid_checkf(!FirstAtom.IsType<char*>(), TEXT("Custom event type cannot be a string literal. It must be a valid flecs ID."));
				solid_checkf(!SecondAtom.IsType<char*>(), TEXT("Custom event type cannot be a string literal. It must be a valid flecs ID."));
				
				InObserverBuilder.event(FFlecsId::MakePair(FirstAtom.Get<FFlecsId>(), SecondAtom.Get<FFlecsId>()));
			}
			else
			{
				FFlecsTermRefAtom_Internal TermAtom = CustomEventType.GetFirstTermRef(InFlecsWorld);
				solid_checkf(!TermAtom.IsType<char*>(), TEXT("Custom event type cannot be a string literal. It must be a valid flecs ID."));
				
				InObserverBuilder.event(TermAtom.Get<FFlecsId>());
			}
		}
		else UNLIKELY_ATTRIBUTE
		{
			UE_LOG(LogFlecsCore, Error, TEXT("Custom event type is invalid. Observer will not have an event type set."));
		}
	}
	else
	{
		switch (EventType)
		{
			case EFlecsObserverEvent::OnAdd:
				InObserverBuilder.event(flecs::OnAdd);
				break;
			case EFlecsObserverEvent::OnRemove:
				InObserverBuilder.event(flecs::OnRemove);
				break;
			case EFlecsObserverEvent::OnSet:
				InObserverBuilder.event(flecs::OnSet);
				break;
			case EFlecsObserverEvent::OnDelete:
				InObserverBuilder.event(flecs::OnDelete);
				break;
			case EFlecsObserverEvent::OnDeleteTarget:
				InObserverBuilder.event(flecs::OnDeleteTarget);
				break;
			case EFlecsObserverEvent::OnTableCreate:
				InObserverBuilder.event(flecs::OnTableCreate);
				break;
			case EFlecsObserverEvent::OnTableDelete:
				InObserverBuilder.event(flecs::OnTableDelete);
				break;
			default:
				break;
		}
	}
}
