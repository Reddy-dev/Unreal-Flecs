// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "UnrealFlecsEditor.h"

#include "Editor.h"
#include "PlayInEditorDataTypes.h"
#include "PropertyEditorModule.h"
#include "ToolMenus.h"
#include "Engine/AssetManagerSettings.h"
#include "Engine/AssetManagerTypes.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Framework/Docking/TabManager.h"
#include "Features/IModularFeatures.h"
#include "IRewindDebuggerTrackCreator.h"
#include "TraceServices/ModuleService.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "General/FlecsEditorDeveloperSettings.h"
#include "General/FlecsEditorPerProjectDeveloperSettings.h"
#include "General/FlecsThreadAllocationPolicyBaseAsset.h"

#include "UnrealFlecsEditorStyle.h"
#include "Widgets/EntityHandle/FlecsIdCustomization.h"
#include "Widgets/EntityHandle/FlecsIdPinFactory.h"
#include "Widgets/Explorer/SFlecsExplorer.h"
#include "RewindDebugger/FlecsRewindDebuggerTraceModule.h"
#include "RewindDebugger/FlecsRewindDebuggerTrack.h"

DEFINE_LOG_CATEGORY_STATIC(LogFlecsEditor, Log, All);

#define LOCTEXT_NAMESPACE "FUnrealFlecsEditorModule"

namespace
{
	const FName FlecsExplorerTabName(TEXT("UnrealFlecs.Explorer"));
} // namespace

FUnrealFlecsEditorModule::~FUnrealFlecsEditorModule() = default;

void FUnrealFlecsEditorModule::StartupModule()
{
	FUnrealFlecsEditorStyle::Initialize();

	FlecsRewindDebuggerTraceModule =
		MakeUnique<UE::Flecs::RewindDebugger::FTraceModule>();
	FlecsRewindDebuggerTrackCreator =
		MakeUnique<UE::Flecs::RewindDebugger::FTrackCreator>();
	IModularFeatures::Get().RegisterModularFeature(
		TraceServices::ModuleFeatureName,
		FlecsRewindDebuggerTraceModule.Get());
	IModularFeatures::Get().RegisterModularFeature(
		RewindDebugger::IRewindDebuggerTrackCreator::ModularFeatureName,
		FlecsRewindDebuggerTrackCreator.Get());

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FlecsExplorerTabName,
		FOnSpawnTab::CreateRaw(this, &FUnrealFlecsEditorModule::SpawnExplorerTab)
		)
		.SetDisplayName(INVTEXT("Flecs Explorer"))
		.SetTooltipText(INVTEXT("Inspect the active Flecs world"))
		.SetMenuType(ETabSpawnerMenuType::Hidden)
		.SetIcon(FSlateIcon("UnrealFlecsEditorStyle", "UnrealFlecs.FlecsEditor.FlecsLogo"));

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this,
		&FUnrealFlecsEditorModule::RegisterExplorerMenuExtension));
	PostPIEStartedHandle = FEditorDelegates::PostPIEStarted.AddRaw(
		this,
		&FUnrealFlecsEditorModule::HandlePostPIEStarted
		);

	FPropertyEditorModule& PropertyEditorModule
		= FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyEditorModule.RegisterCustomPropertyTypeLayout("FlecsId",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(
			&FFlecsIdCustomization::MakeInstance
			)
		);

	FCoreDelegates::GetOnPostEngineInit().AddLambda([this]()
	{
		AddPrimaryAssetTypes();
	});

	PropertyEditorModule.NotifyCustomizationModuleChanged();

	 FlecsIdPinFactory = MakeShared<FFlecsIdPinFactory>();
	 FEdGraphUtilities::RegisterVisualPinFactory(FlecsIdPinFactory);
}

void FUnrealFlecsEditorModule::ShutdownModule()
{
	if (FlecsRewindDebuggerTrackCreator)
	{
		IModularFeatures::Get().UnregisterModularFeature(
			RewindDebugger::IRewindDebuggerTrackCreator::ModularFeatureName,
			FlecsRewindDebuggerTrackCreator.Get());
		FlecsRewindDebuggerTrackCreator.Reset();
	}

	if (FlecsRewindDebuggerTraceModule)
	{
		IModularFeatures::Get().UnregisterModularFeature(
			TraceServices::ModuleFeatureName,
			FlecsRewindDebuggerTraceModule.Get());
		FlecsRewindDebuggerTraceModule.Reset();
	}

	FEditorDelegates::PostPIEStarted.Remove(PostPIEStartedHandle);

	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FlecsExplorerTabName);

		if (ExplorerTab.IsValid())
		{
			ExplorerTab.Pin()->RequestCloseTab();
			ExplorerTab.Reset();
		}
	}

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout("FlecsId");

		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}

	 if (FlecsIdPinFactory.IsValid())
	 {
	 	FEdGraphUtilities::UnregisterVisualPinFactory(FlecsIdPinFactory);
	 	FlecsIdPinFactory.Reset();
	 }

	FUnrealFlecsEditorStyle::Shutdown();

	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
}

void FUnrealFlecsEditorModule::RegisterExplorerMenuExtension()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.ModesToolBar");

	FToolMenuSection& Section = Menu->FindOrAddSection("Content");

	Section.AddEntry(FToolMenuEntry::InitToolBarButton(
		"OpenFlecsExplorer", FUIAction(
			FExecuteAction::CreateRaw(this, &FUnrealFlecsEditorModule::OpenExplorer)
		),
		INVTEXT("Open Flecs Explorer"),
		INVTEXT("Open Flecs Explorer in the editor"),
		FSlateIcon("UnrealFlecsEditorStyle", "UnrealFlecs.FlecsEditor.FlecsLogo")
	));
}

void FUnrealFlecsEditorModule::OpenExplorer()
{
	const UFlecsEditorPerProjectDeveloperSettings* PerProjectSettings =
		GetDefault<UFlecsEditorPerProjectDeveloperSettings>();

	if (PerProjectSettings && PerProjectSettings->bOpenFlecsExplorerExternally)
	{
		OpenExplorerExternally();
		return;
	}

	OpenExplorerTab();
}

void FUnrealFlecsEditorModule::OpenExplorerTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FTabId(FlecsExplorerTabName));
}

void FUnrealFlecsEditorModule::OpenExplorerExternally()
{
	int32 InstanceCount = 1;

	if (GEditor)
	{
		const TOptional<FPlayInEditorSessionInfo> PIEInfo = GEditor->GetPlayInEditorSessionInfo();
		if (PIEInfo.IsSet())
		{
			InstanceCount = FMath::Max(PIEInfo->PIEInstanceCount, 1);
		}
	}

	const UFlecsEditorDeveloperSettings* EditorSettings = GetDefault<UFlecsEditorDeveloperSettings>();
	if (!ensureMsgf(EditorSettings, TEXT("Failed to get Flecs Editor Developer Settings.")))
	{
		return;
	}

	const FFlecsEditorExplorerURL ExplorerURL = EditorSettings->GetFlecsExplorerURL();
	for (int32 Index = 0; Index < InstanceCount; ++Index)
	{
		const FString TargetURL = ExplorerURL.ToURLString(static_cast<uint16>(Index));
		FPlatformProcess::LaunchURL(*TargetURL, nullptr, nullptr);
	}
}

void FUnrealFlecsEditorModule::HandlePostPIEStarted(MAYBE_UNUSED bool bInIsSimulating)
{
	const UFlecsEditorPerProjectDeveloperSettings* PerProjectSettings =
		GetDefault<UFlecsEditorPerProjectDeveloperSettings>();

	if (PerProjectSettings && PerProjectSettings->bOpenFlecsExplorerOnPlay)
	{
		OpenExplorer();
	}
}

TSharedRef<SDockTab> FUnrealFlecsEditorModule::SpawnExplorerTab(MAYBE_UNUSED const FSpawnTabArgs& InSpawnTabArgs)
{
	return SAssignNew(ExplorerTab, SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SFlecsExplorer)
		];
}

void FUnrealFlecsEditorModule::AddPrimaryAssetTypes() const
{
	const UFlecsEditorDeveloperSettings* EditorSettings = GetDefault<UFlecsEditorDeveloperSettings>();

	if UNLIKELY_IF(!ensureMsgf(EditorSettings, TEXT("Failed to get Flecs Editor Developer Settings.")))
	{
		return;
	}

	if (EditorSettings->bIgnoreThreadAllocationPolicyWarning)
	{
		return;
	}

	const UAssetManagerSettings* Settings = GetDefault<UAssetManagerSettings>();

	if UNLIKELY_IF(!ensureMsgf(Settings, TEXT("Failed to get Asset Manager Settings.")))
	{
		return;
	}

	const bool bAlreadyRegistered = Settings->PrimaryAssetTypesToScan.ContainsByPredicate(
		[](const FPrimaryAssetTypeInfo& Info)
		{
			return Info.PrimaryAssetType == FName("FlecsThreadAllocationPolicy");
		});

	if (bAlreadyRegistered)
	{
		return;
	}

	TSharedRef<TWeakPtr<SNotificationItem>> WeakNotification = MakeShared<TWeakPtr<SNotificationItem>>();

	FNotificationInfo Info(LOCTEXT("ThreadPolicyNotRegistered",
		"FlecsThreadAllocationPolicy is not registered in the Asset Manager. "
		"Packaged builds will not cook thread policy assets."));
	Info.bFireAndForget = false;
	Info.bUseSuccessFailIcons = true;

	Info.ButtonDetails.Add(FNotificationButtonInfo(
		LOCTEXT("AddToAssetManager", "Add to Asset Manager"),
		LOCTEXT("AddToAssetManagerTooltip", "Adds FlecsThreadAllocationPolicy to DefaultGame.ini"),
		FSimpleDelegate::CreateLambda([WeakNotification]()
		{
			UAssetManagerSettings* MutableSettings = GetMutableDefault<UAssetManagerSettings>();

			FPrimaryAssetTypeInfo TypeInfo;
			TypeInfo.PrimaryAssetType = FName("FlecsThreadAllocationPolicy");
			TypeInfo.SetAssetBaseClass(UFlecsThreadAllocationPolicyBaseAsset::StaticClass());
			TypeInfo.bHasBlueprintClasses = false;
			TypeInfo.Rules.bApplyRecursively = true;
			TypeInfo.Rules.CookRule = EPrimaryAssetCookRule::AlwaysCook;

			MutableSettings->PrimaryAssetTypesToScan.Add(TypeInfo);
			MutableSettings->TryUpdateDefaultConfigFile();

			UE_LOG(LogFlecsEditor, Log, TEXT("Added FlecsThreadAllocationPolicy to Asset Manager settings."));

			if (const TSharedPtr<SNotificationItem> Pinned = WeakNotification->Pin())
			{
				Pinned->SetCompletionState(SNotificationItem::CS_Success);
				Pinned->Fadeout();
			}
		}),
		SNotificationItem::CS_None
	));

	Info.ButtonDetails.Add(FNotificationButtonInfo(
		LOCTEXT("IgnoreForNow", "Ignore for Now"),
		LOCTEXT("IgnoreForNowTooltip", "Dismiss this warning until the next editor session"),
		FSimpleDelegate::CreateLambda([WeakNotification]()
		{
			if (TSharedPtr<SNotificationItem> Pinned = WeakNotification->Pin())
			{
				Pinned->SetCompletionState(SNotificationItem::CS_None);
				Pinned->Fadeout();
			}
		}),
		SNotificationItem::CS_None
	));

	/*Info.ButtonDetails.Add(FNotificationButtonInfo(
		LOCTEXT("IgnorePermanently", "Ignore Permanently"),
		LOCTEXT("IgnorePermanentlyTooltip", "Never show this warning again for this project"),
		FSimpleDelegate::CreateLambda([WeakNotification]()
		{
			UFlecsEditorDeveloperSettings* MutableEditorSettings = GetMutableDefault<UFlecsEditorDeveloperSettings>();
			MutableEditorSettings->bIgnoreThreadAllocationPolicyWarning = true;
			MutableEditorSettings->TryUpdateDefaultConfigFile();

			UE_LOG(LogFlecsEditor, Log, TEXT("Suppressed FlecsThreadAllocationPolicy Asset Manager warning."));

			if (TSharedPtr<SNotificationItem> Pinned = WeakNotification->Pin())
			{
				Pinned->SetCompletionState(SNotificationItem::CS_None);
				Pinned->Fadeout();
			}
		}),
		SNotificationItem::CS_None
	));*/

	*WeakNotification = FSlateNotificationManager::Get().AddNotification(Info);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUnrealFlecsEditorModule, UnrealFlecsEditor)
