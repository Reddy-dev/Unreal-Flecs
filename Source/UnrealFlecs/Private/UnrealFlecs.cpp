// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "UnrealFlecs.h"

#include "Misc/CoreDelegates.h"

#include "General/FlecsOSAPI.h"
#include "Entities/FlecsDefaultEntityEngine.h"
#include "General/FlecsModuleRegistry.h"
#include "Networking/FlecsReplicationTransportBase.h"

#define LOCTEXT_NAMESPACE "FUnrealFlecsModule"

namespace UE::Flecs
{
	// ReSharper disable once CppDeclaratorNeverUsed
	static FOSApiInitializer OSApiInitializer;
} // namespace UE::Flecs

void FUnrealFlecsModule::StartupModule()
{
	UE::Flecs::FFlecsModuleRegistry::Get().RegisterUnrealFlecsModule("UnrealFlecs");

	FFlecsReplicationInterestPolicyRegistry::RegisterPolicy(
		MakeUnique<FFlecsEveryoneReplicationInterestPolicy>());
	FFlecsReplicationInterestPolicyRegistry::RegisterPolicy(
		MakeUnique<FFlecsOwnerReplicationInterestPolicy>());
	FFlecsReplicationInterestPolicyRegistry::RegisterPolicy(
		MakeUnique<FFlecsTeamReplicationInterestPolicy>());
	FFlecsReplicationInterestPolicyRegistry::RegisterPolicy(
		MakeUnique<FFlecsZoneReplicationInterestPolicy>());
	
	FCoreDelegates::GetOnPostEngineInit().AddLambda([]()
	{
		FFlecsDefaultEntityEngine::Get().Initialize();
	});
	
	
}

void FUnrealFlecsModule::ShutdownModule()
{
	FFlecsReplicationInterestPolicyRegistry::UnregisterPolicy(
		FFlecsReplicationInterestPolicyNames::Everyone());
	FFlecsReplicationInterestPolicyRegistry::UnregisterPolicy(
		FFlecsReplicationInterestPolicyNames::Owner());
	FFlecsReplicationInterestPolicyRegistry::UnregisterPolicy(
		FFlecsReplicationInterestPolicyNames::Team());
	FFlecsReplicationInterestPolicyRegistry::UnregisterPolicy(
		FFlecsReplicationInterestPolicyNames::Zone());
}

#undef LOCTEXT_NAMESPACE // "FUnrealFlecsModule"
	
IMPLEMENT_MODULE(FUnrealFlecsModule, UnrealFlecs)
