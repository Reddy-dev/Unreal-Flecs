// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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
	void OpenExplorerTab();
	TSharedRef<SDockTab> SpawnExplorerTab(const FSpawnTabArgs& InSpawnTabArgs);
	void AddPrimaryAssetTypes() const;

	TSharedPtr<FFlecsIdPinFactory> FlecsIdPinFactory;
	TWeakPtr<SDockTab> ExplorerTab;

}; // class FUnrealFlecsEditorModule
