// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Systems/FlecsReplicationQueueSystem.h"

#include "Networking/Subsystem/FlecsNetworkSubsystemSingleton.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationQueueSystem)

UFlecsReplicationQueueSystem::UFlecsReplicationQueueSystem()
{
}

void UFlecsReplicationQueueSystem::BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*>,
	TFlecsSystemBuilder<>& InBuilder) const
{
	InBuilder
		.With<const FFlecsNetworkSubsystemSingleton>();
}

void UFlecsReplicationQueueSystem::EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*>,
	flecs::iter& InIterator, const FFlecsId)
{
	const TSolidNotNull<UFlecsNetworkWorldSubsystem*> NetworkSubsystem =
		InIterator.field_at<const FFlecsNetworkSubsystemSingleton>(0, 0).GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();

	NetworkSubsystem->ApplyQueuedReplicationUpdates();
}

EFlecsObjectRegistrationNetworkFlags UFlecsReplicationQueueSystem::GetObjectRegistrationNetworkFlags() const
{
	return EFlecsObjectRegistrationNetworkFlags::All;
}
