// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "UnrealFlecs.h"

#include "Misc/CoreDelegates.h"

#include "Iris/ReplicationSystem/NetObjectFactoryRegistry.h"

#include "General/FlecsOSAPI.h"
#include "Entities/FlecsDefaultEntityEngine.h"
#include "General/FlecsModuleRegistry.h"
#include "Networking/Layout/FlecsLayoutReplicatorNetFactory.h"

#define LOCTEXT_NAMESPACE "FUnrealFlecsModule"

namespace UE::Flecs
{
	// ReSharper disable once CppDeclaratorNeverUsed
	static FOSApiInitializer OSApiInitializer;
} // namespace UE::Flecs

void FUnrealFlecsModule::StartupModule()
{
	UE::Flecs::FFlecsModuleRegistry::Get().RegisterUnrealFlecsModule("UnrealFlecs");
	UE::Net::FNetObjectFactoryRegistry::RegisterFactory(
		UFlecsLayoutReplicatorNetFactory::StaticClass(),
		UFlecsLayoutReplicatorNetFactory::GetFactoryName());
	/*FFlecsReplicationInterestPolicyRegistry::RegisterPolicy(MakeUnique<FFlecsEveryoneReplicationInterestPolicy>());
	FFlecsReplicationInterestPolicyRegistry::RegisterPolicy(MakeUnique<FFlecsOwnerReplicationInterestPolicy>());
	FFlecsReplicationInterestPolicyRegistry::RegisterPolicy(MakeUnique<FFlecsSpatialCellReplicationInterestPolicy>());*/
	
	FCoreDelegates::GetOnPostEngineInit().AddLambda([]()
	{
		FFlecsDefaultEntityEngine::Get().Initialize();
	});
	
}

void FUnrealFlecsModule::ShutdownModule()
{
	UE::Net::FNetObjectFactoryRegistry::UnregisterFactory(
		UFlecsLayoutReplicatorNetFactory::GetFactoryName());

	/*FFlecsReplicationInterestPolicyRegistry::UnregisterPolicy(FFlecsReplicationInterestPolicyNames::SpatialCell);
	FFlecsReplicationInterestPolicyRegistry::UnregisterPolicy(FFlecsReplicationInterestPolicyNames::Owner);
	FFlecsReplicationInterestPolicyRegistry::UnregisterPolicy(FFlecsReplicationInterestPolicyNames::Everyone);*/
}

#undef LOCTEXT_NAMESPACE // "FUnrealFlecsModule"
	
IMPLEMENT_MODULE(FUnrealFlecsModule, UnrealFlecs)
