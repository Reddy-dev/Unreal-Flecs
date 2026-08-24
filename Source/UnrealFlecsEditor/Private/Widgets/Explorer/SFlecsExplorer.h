// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SBox;
class SHorizontalBox;
class SWebBrowser;

struct FFlecsExplorerTarget
{
	uint16 InstanceIndex = 0;
	FText Label;
	TSharedPtr<SWebBrowser> Browser;
}; // struct FFlecsExplorerTarget

class SFlecsExplorer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFlecsExplorer)
	{
	}
	SLATE_END_ARGS()

	~SFlecsExplorer();

	void Construct(const FArguments& InArgs);

private:
	void RefreshTargets();
	void RebuildTargetTabs();
	void ShowSelectedTarget();
	void EnsureTargetBrowser(const TSharedPtr<FFlecsExplorerTarget>& InTarget);
	void HandlePIEStateChanged(bool bInIsSimulating);
	FReply SelectTarget(TSharedPtr<FFlecsExplorerTarget> InTarget);
	FString GetTargetURL(const TSharedPtr<FFlecsExplorerTarget>& InTarget) const;
	FReply ReloadExplorer();
	FReply OpenExplorerExternally();
	bool HandleBeforePopup(FString InURL, FString InFrame);

	TArray<TSharedPtr<FFlecsExplorerTarget>> Targets;
	TSharedPtr<FFlecsExplorerTarget> SelectedTarget;
	TSharedPtr<SHorizontalBox> TargetTabBar;
	TSharedPtr<SBox> BrowserContainer;
	FDelegateHandle PostPIEStartedHandle;
	FDelegateHandle ShutdownPIEHandle;
	bool bWebBrowserAvailable = false;
}; // class SFlecsExplorer
