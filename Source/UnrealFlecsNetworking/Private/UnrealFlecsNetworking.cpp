// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "UnrealFlecsNetworking.h"

#include "Iris/ReplicationSystem/NetObjectFactoryRegistry.h"

#include "General/FlecsModuleRegistry.h"
#include "Networking/Bridge/FlecsIrisReplicationBridgeNetFactory.h"
#include "Networking/FlecsComponentReplicationDescriptor.h"
#include "Networking/Shards/FlecsNetEntityProxyNetFactory.h"
#include "Properties/FlecsComponentRegistrationHooks.h"

void FUnrealFlecsNetworkingModule::StartupModule()
{
	UE::Flecs::FFlecsModuleRegistry::Get().RegisterUnrealFlecsModule("UnrealFlecsNetworking");
	
	UE::Flecs::FFlecsComponentRegistrationHooks::InstallReplicationHooks(
		this,
		&UE::Flecs::Replication::RegisterComponentDefinition,
		&UE::Flecs::Replication::MarkComponentReplicated);

	UE::Net::FNetObjectFactoryRegistry::RegisterFactory(
		UFlecsIrisReplicationBridgeNetFactory::StaticClass(),
		UFlecsIrisReplicationBridgeNetFactory::GetFactoryName());
	UE::Net::FNetObjectFactoryRegistry::RegisterFactory(
		UFlecsNetEntityProxyNetFactory::StaticClass(),
		UFlecsNetEntityProxyNetFactory::GetFactoryName());
}

void FUnrealFlecsNetworkingModule::ShutdownModule()
{
	UE::Net::FNetObjectFactoryRegistry::UnregisterFactory(
		UFlecsIrisReplicationBridgeNetFactory::GetFactoryName());
	UE::Net::FNetObjectFactoryRegistry::UnregisterFactory(
		UFlecsNetEntityProxyNetFactory::GetFactoryName());

	UE::Flecs::FFlecsComponentRegistrationHooks::UninstallReplicationHooks(this);
}

IMPLEMENT_MODULE(FUnrealFlecsNetworkingModule, UnrealFlecsNetworking)
