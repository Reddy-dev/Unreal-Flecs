// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "UnrealFlecs.h"

#include "Misc/CoreDelegates.h"

#include "General/FlecsOSAPI.h"
#include "Entities/FlecsDefaultEntityEngine.h"
#include "General/FlecsModuleRegistry.h"

#define LOCTEXT_NAMESPACE "FUnrealFlecsModule"

namespace UE::Flecs
{
	// ReSharper disable once CppDeclaratorNeverUsed
	static FOSApiInitializer OSApiInitializer;
} // namespace UE::Flecs

void FUnrealFlecsModule::StartupModule()
{
	UE::Flecs::FFlecsModuleRegistry::Get().RegisterUnrealFlecsModule("UnrealFlecs");
	
	FCoreDelegates::OnPostEngineInit.AddLambda([]()
	{
		FFlecsDefaultEntityEngine::Get().Initialize();
	});
	
	
}

void FUnrealFlecsModule::ShutdownModule()
{
	
}

#undef LOCTEXT_NAMESPACE // "FUnrealFlecsModule"
	
IMPLEMENT_MODULE(FUnrealFlecsModule, UnrealFlecs)