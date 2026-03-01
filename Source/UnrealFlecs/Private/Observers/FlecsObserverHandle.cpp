// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Observers/FlecsObserverHandle.h"

#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsObserverHandle)

FFlecsObserverHandle::FFlecsObserverHandle(const TSolidNotNull<const UFlecsWorld*> InWorld,
	const FFlecsObserverDefinition& InObserverBuilder, const FString& InObserverName)
{
	flecs::observer_builder<> Builder(InWorld->World, TCHAR_TO_UTF8(*InObserverName));
	InObserverBuilder.ApplyToObserver(InWorld, Builder);
	Entity = Builder.build();
}
