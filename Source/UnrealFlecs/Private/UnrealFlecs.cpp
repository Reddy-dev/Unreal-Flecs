// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "UnrealFlecs.h"

#include "Misc/CoreDelegates.h"

#include "General/FlecsOSAPI.h"
#include "Entities/FlecsDefaultEntityEngine.h"
#include "General/FlecsModuleRegistry.h"
#include "Debugging/FlecsRewindDebuggerRuntimeExtension.h"
#include "Features/IModularFeatures.h"
#include "RewindDebuggerRuntimeInterface/IRewindDebuggerRuntimeExtension.h"

#define LOCTEXT_NAMESPACE "FUnrealFlecsModule"

namespace UE::Flecs
{
	// ReSharper disable once CppDeclaratorNeverUsed
	static FOSApiInitializer OSApiInitializer;
} // namespace UE::Flecs

FUnrealFlecsModule::~FUnrealFlecsModule() = default;

void FUnrealFlecsModule::StartupModule()
{
	UE::Flecs::FFlecsModuleRegistry::Get().RegisterUnrealFlecsModule("UnrealFlecs");
	RewindDebuggerRuntimeExtension = MakeUnique<FFlecsRewindDebuggerRuntimeExtension>();
	IModularFeatures::Get().RegisterModularFeature(
		IRewindDebuggerRuntimeExtension::ModularFeatureName,
		RewindDebuggerRuntimeExtension.Get());
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
	if (RewindDebuggerRuntimeExtension)
	{
		IModularFeatures::Get().UnregisterModularFeature(
			IRewindDebuggerRuntimeExtension::ModularFeatureName,
			RewindDebuggerRuntimeExtension.Get());
		RewindDebuggerRuntimeExtension.Reset();
	}

	/*FFlecsReplicationInterestPolicyRegistry::UnregisterPolicy(FFlecsReplicationInterestPolicyNames::SpatialCell);
	FFlecsReplicationInterestPolicyRegistry::UnregisterPolicy(FFlecsReplicationInterestPolicyNames::Owner);
	FFlecsReplicationInterestPolicyRegistry::UnregisterPolicy(FFlecsReplicationInterestPolicyNames::Everyone);*/
}

#undef LOCTEXT_NAMESPACE // "FUnrealFlecsModule"
	
IMPLEMENT_MODULE(FUnrealFlecsModule, UnrealFlecs)
