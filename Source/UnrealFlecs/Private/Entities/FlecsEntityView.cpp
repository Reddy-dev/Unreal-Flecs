// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Entities/FlecsEntityView.h"

#include "Worlds/FlecsWorldInterfaceObject.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsEntityView)

FFlecsEntityView FFlecsEntityView::GetNullHandle(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld)
{
	return FFlecsEntityView(flecs::entity::null(InWorld->GetNativeFlecsWorld()));
}
