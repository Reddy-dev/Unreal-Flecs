// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Observers/FlecsObserverHandleInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsObserverHandleInterface)

FFlecsEntityHandle IFlecsObserverHandleInterface::GetEntityHandle() const
{
	return GetObserverHandle().GetObserverEntity();
}
