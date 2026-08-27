// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Systems/FlecsReplicationQueueSystem.h"

#include "Networking/Subsystem/FlecsNetworkSubsystemSingleton.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationQueueSystem)

UFlecsReplicationQueueSystem::UFlecsReplicationQueueSystem()
{
	NetworkRegistrationFlags = static_cast<uint8>(EFlecsObjectRegistrationNetworkFlags::Client);
}

void UFlecsReplicationQueueSystem::BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*>,
	TFlecsSystemBuilder<>& InBuilder) const
{
	InBuilder
		.Phase(EFlecsPhaseType::PostUpdate)
		.With<const FFlecsNetworkSubsystemSingleton>();
}

void UFlecsReplicationQueueSystem::RunEachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorldInterfaceObject,
	flecs::iter& InIterator)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_FlecsReplicationQueueSystem_RunEachIterator);
	
	const TSolidNotNull<UFlecsNetworkWorldSubsystem*> NetworkSubsystem =
		InIterator.field_at<const FFlecsNetworkSubsystemSingleton>(0, 0).GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
	
	NetworkSubsystem->ApplyQueuedReplicationUpdates(InWorldInterfaceObject);
}