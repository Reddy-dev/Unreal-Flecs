// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsNetworkWorldSubsystem.h"

#include "Networking/FlecsNetworkSubsystemSingleton.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetworkWorldSubsystem)

UFlecsNetworkWorldSubsystem::UFlecsNetworkWorldSubsystem()
{
}

void UFlecsNetworkWorldSubsystem::OnFlecsWorldInitialized(const TSolidNotNull<UFlecsWorld*> InWorld)
{
	InWorld->Set<FFlecsNetworkSubsystemSingleton>(FFlecsNetworkSubsystemSingleton{ this });
}

FFlecsNetworkId UFlecsNetworkWorldSubsystem::BeginReplicatingEntity(const FFlecsEntityHandle& EntityHandle)
{
	return FFlecsNetworkId();
}

void UFlecsNetworkWorldSubsystem::StopReplicatingEntity(const FFlecsNetworkId& NetworkId)
{
	
}
