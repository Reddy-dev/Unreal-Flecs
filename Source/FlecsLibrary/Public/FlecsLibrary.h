// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FFlecsLibraryModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
}; // class FFlecsLibraryModule
