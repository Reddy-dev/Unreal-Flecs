// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FFlecsIdPinFactory;
class SDockTab;
class FSpawnTabArgs;

namespace UE::Flecs::RewindDebugger
{
	class FTraceModule;
	class FTrackCreator;
}

class FUnrealFlecsEditorModule : public IModuleInterface
{
public:
	virtual ~FUnrealFlecsEditorModule() override;
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
	TUniquePtr<UE::Flecs::RewindDebugger::FTraceModule> FlecsRewindDebuggerTraceModule;
	TUniquePtr<UE::Flecs::RewindDebugger::FTrackCreator> FlecsRewindDebuggerTrackCreator;

}; // class FUnrealFlecsEditorModule
