// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "UnrealFlecsGameFramework.h"

#include "General/FlecsModuleRegistry.h"

void FUnrealFlecsGameFrameworkModule::StartupModule()
{
	UE::Flecs::FFlecsModuleRegistry::Get().RegisterUnrealFlecsModule("UnrealFlecsGameFramework");
}

void FUnrealFlecsGameFrameworkModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FUnrealFlecsGameFrameworkModule, UnrealFlecsGameFramework)
