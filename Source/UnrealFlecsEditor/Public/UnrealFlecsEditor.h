// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FFlecsIdPinFactory;
class SDockTab;
class FSpawnTabArgs;

class FUnrealFlecsEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterExplorerMenuExtension();
	void OpenExplorer();
	void OpenExplorerTab();
	void OpenExplorerExternally();
	void HandlePostPIEStarted(bool bInIsSimulating);
	TSharedRef<SDockTab> SpawnExplorerTab(const FSpawnTabArgs& InSpawnTabArgs);
	void AddPrimaryAssetTypes() const;

	TSharedPtr<FFlecsIdPinFactory> FlecsIdPinFactory;
	TWeakPtr<SDockTab> ExplorerTab;
	FDelegateHandle PostPIEStartedHandle;

}; // class FUnrealFlecsEditorModule
