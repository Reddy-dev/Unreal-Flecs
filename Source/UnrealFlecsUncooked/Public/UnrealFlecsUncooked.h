#pragma once


#include "Modules/ModuleManager.h"

class FUnrealFlecsUncookedModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
