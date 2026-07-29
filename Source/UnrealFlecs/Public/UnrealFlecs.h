// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FFlecsRewindDebuggerRuntimeExtension;

class FUnrealFlecsModule : public IModuleInterface
{
public:
	virtual ~FUnrealFlecsModule() override;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TUniquePtr<FFlecsRewindDebuggerRuntimeExtension> RewindDebuggerRuntimeExtension;
	
}; // class FUnrealFlecsModule
