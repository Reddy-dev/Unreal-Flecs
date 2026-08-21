// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Observers/FlecsReplicatedComponentObservers.h"

#include "Networking/Subsystem/FlecsNetworkSubsystemSingleton.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"
#include "Networking/FlecsReplicatedEntityComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicatedComponentObservers)

UFlecsReplicatedComponentObservers::UFlecsReplicatedComponentObservers()
{
	NetworkRegistrationFlags = static_cast<uint8>(EFlecsObjectRegistrationNetworkFlags::Server);
}

void UFlecsReplicatedComponentObservers::BuildObserver(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
                                                       TFlecsObserverBuilder<>& InOutBuilder) const
{
	InOutBuilder
		.Event(flecs::OnAdd)
		.Event(flecs::OnRemove)
		.With<FFlecsReplicatedEntityComponent>() // 0
		.With<const FFlecsNetworkSubsystemSingleton>() // 1
		.YieldExisting();
}

void UFlecsReplicatedComponentObservers::EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	flecs::iter& InIterator, const FFlecsId InIndex)
{
	const auto NetworkSubsystem = InIterator.field<const FFlecsNetworkSubsystemSingleton>(1);
	
	const TSolidNotNull<UFlecsNetworkWorldSubsystem*> NetworkSubsystemPtr = NetworkSubsystem->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
	const FFlecsEntityHandle EntityHandle = InIterator.entity(InIndex.GetId());

	if (InIterator.event() == flecs::OnRemove)
	{
		NetworkSubsystemPtr->StopReplicatingEntity(EntityHandle);
		return;
	}

	NetworkSubsystemPtr->BeginReplicatingEntity(EntityHandle);
}
