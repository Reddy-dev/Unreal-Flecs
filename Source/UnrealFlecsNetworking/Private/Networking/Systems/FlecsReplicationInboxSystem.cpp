// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Systems/FlecsReplicationInboxSystem.h"

#include "Networking/FlecsReplicationInbox.h"
#include "Networking/Subsystem/FlecsNetworkSubsystemSingleton.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"
#include "Systems/FlecsSystemPhaseInput.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationInboxSystem)

UFlecsReplicationInboxSystem::UFlecsReplicationInboxSystem()
{
}

void UFlecsReplicationInboxSystem::BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*>,
	TFlecsSystemBuilder<>& InBuilder) const
{
	InBuilder
		.With<FFlecsReplicationInbox&>() // 0
		.With<const FFlecsNetworkSubsystemSingleton>(); // 1
}

void UFlecsReplicationInboxSystem::EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*>,
	flecs::iter& InIterator, const FFlecsId)
{
	FFlecsReplicationInbox& Inbox = InIterator.field_at<FFlecsReplicationInbox>(0, 0);
	const TSolidNotNull<UFlecsNetworkWorldSubsystem*> NetworkSubsystem = InIterator.field_at<const FFlecsNetworkSubsystemSingleton>(1, 0)
			.GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();

	NetworkSubsystem->ApplyReplicationInbox(Inbox);
}

EFlecsObjectRegistrationNetworkFlags UFlecsReplicationInboxSystem::GetObjectRegistrationNetworkFlags() const
{
	return EFlecsObjectRegistrationNetworkFlags::All;
}
