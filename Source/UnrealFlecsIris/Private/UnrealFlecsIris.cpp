// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "UnrealFlecsIris.h"

#include "Iris/ReplicationSystem/NetObjectFactoryRegistry.h"
//#include "Networking/FlecsIrisReplicationTransport.h"
#include "Networking/FlecsIrisShardObjectFactory.h"

void FUnrealFlecsIrisModule::StartupModule()
{
	/*FFlecsReplicationTransportRegistry::RegisterProvider(TEXT("Iris"),
		UFlecsIrisReplicationTransport::StaticClass());*/
	UE::Net::FNetObjectFactoryRegistry::RegisterFactory(UFlecsIrisShardObjectFactory::StaticClass(),
		UFlecsIrisShardObjectFactory::GetFactoryName());
}

void FUnrealFlecsIrisModule::ShutdownModule()
{
	UE::Net::FNetObjectFactoryRegistry::UnregisterFactory(UFlecsIrisShardObjectFactory::GetFactoryName());
	/*FFlecsReplicationTransportRegistry::UnregisterProvider(TEXT("Iris"));*/
}

IMPLEMENT_MODULE(FUnrealFlecsIrisModule, UnrealFlecsIris)
