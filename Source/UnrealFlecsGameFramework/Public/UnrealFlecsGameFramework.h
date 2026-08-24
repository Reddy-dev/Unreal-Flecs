// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FUnrealFlecsGameFrameworkModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

}; // class FUnrealFlecsGameFrameworkModule
