// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FUnrealFlecsNetworkingModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

}; // class FUnrealFlecsNetworkingModule
