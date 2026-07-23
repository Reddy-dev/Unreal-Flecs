// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Widgets/Explorer/SFlecsExplorer.h"

#include "Editor.h"
#include "PlayInEditorDataTypes.h"
#include "SWebBrowser.h"
#include "WebBrowserModule.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "General/FlecsEditorDeveloperSettings.h"
#include "General/FlecsExplorerURLSettings.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SFlecsExplorer::Construct(const FArguments& InArgs)
{
	(void)InArgs;

	PostPIEStartedHandle = FEditorDelegates::PostPIEStarted.AddSP(
		SharedThis(this),
		&SFlecsExplorer::HandlePIEStateChanged
		);
	ShutdownPIEHandle = FEditorDelegates::ShutdownPIE.AddSP(
		SharedThis(this),
		&SFlecsExplorer::HandlePIEStateChanged
		);

	bWebBrowserAvailable = IWebBrowserModule::Get().IsWebModuleAvailable();
	RefreshTargets();

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SScrollBox)
				.Orientation(Orient_Horizontal)
				+ SScrollBox::Slot()
				[
					SAssignNew(TargetTabBar, SHorizontalBox)
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(6.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(INVTEXT("Reload"))
				.ToolTipText(INVTEXT("Reload the embedded Flecs Explorer"))
				.OnClicked(this, &SFlecsExplorer::ReloadExplorer)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(INVTEXT("Open Externally"))
				.ToolTipText(INVTEXT("Open the selected Flecs Explorer instance in the system browser"))
				.OnClicked(this, &SFlecsExplorer::OpenExplorerExternally)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator)
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(BrowserContainer, SBox)
		]
	];

	RebuildTargetTabs();
	ShowSelectedTarget();
}

SFlecsExplorer::~SFlecsExplorer()
{
	FEditorDelegates::PostPIEStarted.Remove(PostPIEStartedHandle);
	FEditorDelegates::ShutdownPIE.Remove(ShutdownPIEHandle);
}

void SFlecsExplorer::RefreshTargets()
{
	uint16 PreviousInstanceIndex = SelectedTarget.IsValid() ? SelectedTarget->InstanceIndex : 0;
	int32 InstanceCount = 1;
	TMap<uint16, TSharedPtr<SWebBrowser>> ExistingBrowsers;

	for (const TSharedPtr<FFlecsExplorerTarget>& Target : Targets)
	{
		if (Target.IsValid() && Target->Browser.IsValid())
		{
			ExistingBrowsers.Add(Target->InstanceIndex, Target->Browser);
		}
	}

	if (GEditor)
	{
		const TOptional<FPlayInEditorSessionInfo> PIEInfo = GEditor->GetPlayInEditorSessionInfo();
		if (PIEInfo.IsSet())
		{
			InstanceCount = FMath::Max(PIEInfo->PIEInstanceCount, 1);
		}
	}

	Targets.Reset(InstanceCount);

	const UFlecsEditorDeveloperSettings* EditorSettings = GetDefault<UFlecsEditorDeveloperSettings>();
	const FFlecsEditorExplorerURL URLSettings = EditorSettings
		? EditorSettings->GetFlecsExplorerURL()
		: FFlecsEditorExplorerURL();

	for (int32 Index = 0; Index < InstanceCount; ++Index)
	{
		const uint16 InstanceIndex = static_cast<uint16>(Index);
		const uint32 InstancePort = URLSettings.Port
			+ (URLSettings.IncrementPortForClientInstances ? InstanceIndex : 0);
		FText TargetName = FText::Format(
			INVTEXT("Instance {0}"),
			FText::AsNumber(InstanceIndex)
			);

		if (GEngine)
		{
			for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
			{
				const UWorld* World = WorldContext.World();
				if (WorldContext.WorldType != EWorldType::PIE
					|| WorldContext.PIEInstance != Index
					|| !World)
				{
					continue;
				}

				switch (World->GetNetMode())
				{
				case NM_Client:
					TargetName = FText::Format(
						INVTEXT("Client {0}"),
						FText::AsNumber(InstanceIndex)
						);
					break;

				case NM_DedicatedServer:
					TargetName = INVTEXT("Dedicated Server");
					break;

				case NM_ListenServer:
					TargetName = INVTEXT("Listen Server");
					break;

				case NM_Standalone:
					TargetName = INVTEXT("Standalone");
					break;

				default:
					break;
				}

				break;
			}
		}

		TSharedPtr<FFlecsExplorerTarget> Target = MakeShared<FFlecsExplorerTarget>();
		Target->InstanceIndex = InstanceIndex;
		Target->Browser = ExistingBrowsers.FindRef(InstanceIndex);
		Target->Label = FText::Format(
			INVTEXT("{0} ({1}:{2})"),
			TargetName,
			FText::FromString(URLSettings.Host),
			FText::AsNumber(InstancePort)
			);
		Targets.Add(MoveTemp(Target));
	}

	PreviousInstanceIndex = FMath::Min<uint16>(
		PreviousInstanceIndex,
		static_cast<uint16>(Targets.Num() - 1)
		);
	SelectedTarget = Targets[PreviousInstanceIndex];

	if (TargetTabBar.IsValid())
	{
		RebuildTargetTabs();
	}

	if (BrowserContainer.IsValid())
	{
		ShowSelectedTarget();

		if (SelectedTarget->Browser.IsValid())
		{
			SelectedTarget->Browser->LoadURL(GetTargetURL(SelectedTarget));
		}
	}
}

void SFlecsExplorer::RebuildTargetTabs()
{
	if (!TargetTabBar.IsValid())
	{
		return;
	}

	TargetTabBar->ClearChildren();

	for (const TSharedPtr<FFlecsExplorerTarget>& Target : Targets)
	{
		const bool bIsSelected = Target == SelectedTarget;
		const FText TabLabel = bIsSelected
			? FText::Format(INVTEXT("[Active] {0}"), Target->Label)
			: Target->Label;

		TargetTabBar->AddSlot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SButton)
				.Text(TabLabel)
				.IsEnabled(!bIsSelected)
				.OnClicked(this, &SFlecsExplorer::SelectTarget, Target)
		];
	}
}

void SFlecsExplorer::ShowSelectedTarget()
{
	if (!BrowserContainer.IsValid())
	{
		return;
	}

	if (!bWebBrowserAvailable)
	{
		BrowserContainer->SetContent(
			SNew(SBorder)
			.Padding(16.0f)
			[
				SNew(STextBlock)
				.Text(INVTEXT(
					"The Unreal Web Browser module is unavailable on this platform.\n"
					"Use Open Externally to launch Flecs Explorer in the system browser."
					))
				.AutoWrapText(true)
			]
			);
		return;
	}

	if (!SelectedTarget.IsValid())
	{
		BrowserContainer->SetContent(
			SNew(STextBlock)
				.Text(INVTEXT("No Flecs Explorer target is available."))
			);
		return;
	}

	EnsureTargetBrowser(SelectedTarget);
	BrowserContainer->SetContent(SelectedTarget->Browser.ToSharedRef());
}

void SFlecsExplorer::EnsureTargetBrowser(
	const TSharedPtr<FFlecsExplorerTarget>& InTarget
	)
{
	if (!InTarget.IsValid() || InTarget->Browser.IsValid())
	{
		return;
	}

	InTarget->Browser = SNew(SWebBrowser)
		.InitialURL(GetTargetURL(InTarget))
		.ShowAddressBar(false)
		.ShowControls(false)
		.ShowErrorMessage(true)
		.ShowInitialThrobber(true)
		.BrowserFrameRate(60)
		.OnBeforePopup(this, &SFlecsExplorer::HandleBeforePopup);
}

void SFlecsExplorer::HandlePIEStateChanged(bool bInIsSimulating)
{
	(void)bInIsSimulating;

	RefreshTargets();
}

FReply SFlecsExplorer::SelectTarget(TSharedPtr<FFlecsExplorerTarget> InTarget)
{
	if (!InTarget.IsValid() || InTarget == SelectedTarget)
	{
		return FReply::Handled();
	}

	SelectedTarget = MoveTemp(InTarget);
	RebuildTargetTabs();
	ShowSelectedTarget();
	return FReply::Handled();
}

FString SFlecsExplorer::GetTargetURL(
	const TSharedPtr<FFlecsExplorerTarget>& InTarget
	) const
{
	const UFlecsEditorDeveloperSettings* EditorSettings = GetDefault<UFlecsEditorDeveloperSettings>();
	if (!EditorSettings)
	{
		return FFlecsEditorExplorerURL().ToURLString();
	}

	const uint16 InstanceIndex = InTarget.IsValid() ? InTarget->InstanceIndex : 0;
	return EditorSettings->GetFlecsExplorerURL().ToURLString(InstanceIndex);
}

FReply SFlecsExplorer::ReloadExplorer()
{
	if (SelectedTarget.IsValid() && SelectedTarget->Browser.IsValid())
	{
		SelectedTarget->Browser->Reload();
	}

	return FReply::Handled();
}

FReply SFlecsExplorer::OpenExplorerExternally()
{
	const FString TargetURL = GetTargetURL(SelectedTarget);
	FPlatformProcess::LaunchURL(*TargetURL, nullptr, nullptr);
	return FReply::Handled();
}

bool SFlecsExplorer::HandleBeforePopup(FString InURL, FString InFrame)
{
	(void)InFrame;

	if (!InURL.IsEmpty())
	{
		FPlatformProcess::LaunchURL(*InURL, nullptr, nullptr);
	}

	return true;
}
