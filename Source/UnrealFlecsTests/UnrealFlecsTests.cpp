// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "UnrealFlecsTests.h"

#include "General/FlecsModuleRegistry.h"
#include "UnrealFlecsConfigMacros.h"

#define LOCTEXT_NAMESPACE "FUnrealFlecsTestsModule"

void FUnrealFlecsTestsModule::StartupModule()
{
#if ENABLE_UNREAL_FLECS_TESTS
	UE::Flecs::FFlecsModuleRegistry::Get().RegisterUnrealFlecsModule("UnrealFlecsTests");
#endif // ENABLE_UNREAL_FLECS_TESTS
}

void FUnrealFlecsTestsModule::ShutdownModule()
{
	//UE::Flecs::FFlecsModuleRegistry::Get().("UnrealFlecsTests");
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FUnrealFlecsTestsModule, UnrealFlecsTests)