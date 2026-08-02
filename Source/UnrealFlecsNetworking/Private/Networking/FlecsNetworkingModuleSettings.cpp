// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsNetworkingModuleSettings.h"

#include "Networking/DefaultFlecsNetworkIdGenerator.h"
#include "Networking/Bridge/FlecsIrisReplicationBridge.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetworkingModuleSettings)

void UFlecsNetworkingModuleSettings::PostInitProperties()
{
	Super::PostInitProperties();
	
	NetworkIdGeneratorClass = UDefaultFlecsNetworkIdGenerator::StaticClass();

	ReplicationBridgeClass = UFlecsIrisReplicationBridge::StaticClass();
}
