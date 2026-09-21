// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Observers/FlecsObserverHandle.h"

#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsObserverHandle)

FFlecsObserverHandle::FFlecsObserverHandle(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld,
	const FFlecsObserverDefinition& InObserverBuilder, const FString& InObserverName)
{
	flecs::observer_builder<> Builder(InWorld->GetNativeFlecsWorld(), TCHAR_TO_UTF8(*InObserverName));
	InObserverBuilder.ApplyToObserver(InWorld, Builder);
	Entity = Builder.build();
}

FFlecsObserverHandle::FFlecsObserverHandle(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld,
	const FFlecsObserverDefinition& InObserverBuilder, const FFlecsId InExistingEntity)
{
	solid_checkf(InWorld->IsAlive(InExistingEntity), TEXT("Existing observer entity is not alive."));

	flecs::observer_builder<> Builder(InWorld->GetNativeFlecsWorld());
	Builder._internal_get_desc()->entity = InExistingEntity.GetId();
	InObserverBuilder.ApplyToObserver(InWorld, Builder);
	Entity = Builder.build();
}
