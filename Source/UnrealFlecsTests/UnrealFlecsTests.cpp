// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "UnrealFlecsTests.h"

#include "General/FlecsModuleRegistry.h"

#define LOCTEXT_NAMESPACE "FUnrealFlecsTestsModule"

void FUnrealFlecsTestsModule::StartupModule()
{
	UE::Flecs::FFlecsModuleRegistry::Get().RegisterUnrealFlecsModule("UnrealFlecsTests");
}

void FUnrealFlecsTestsModule::ShutdownModule()
{
	//UE::Flecs::FFlecsModuleRegistry::Get().("UnrealFlecsTests");
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FUnrealFlecsTestsModule, UnrealFlecsTests)