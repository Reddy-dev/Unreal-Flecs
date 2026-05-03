// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "UnrealFlecsEditor.h"

#include "Engine/AssetManagerSettings.h"
#include "Engine/AssetManagerTypes.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "General/FlecsEditorDeveloperSettings.h"
#include "General/FlecsThreadAllocationPolicyBaseAsset.h"

#include "UnrealFlecsEditorStyle.h"
#include "Widgets/EntityHandle/FlecsIdCustomization.h"
#include "Widgets/EntityHandle/FlecsIdPinFactory.h"

DEFINE_LOG_CATEGORY_STATIC(LogFlecsEditor, Log, All);

#define LOCTEXT_NAMESPACE "FUnrealFlecsEditorModule"

void FUnrealFlecsEditorModule::StartupModule()
{
	FUnrealFlecsEditorStyle::Initialize();

    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this,
    	&FUnrealFlecsEditorModule::RegisterExplorerMenuExtension));

	FPropertyEditorModule& PropertyEditorModule
		= FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyEditorModule.RegisterCustomPropertyTypeLayout("FlecsId",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(
			&FFlecsIdCustomization::MakeInstance
			)
		);

	FCoreDelegates::OnPostEngineInit.AddLambda([this]() { AddPrimaryAssetTypes(); });

	PropertyEditorModule.NotifyCustomizationModuleChanged();

	 FlecsIdPinFactory = MakeShared<FFlecsIdPinFactory>();
	 FEdGraphUtilities::RegisterVisualPinFactory(FlecsIdPinFactory);
}

void FUnrealFlecsEditorModule::ShutdownModule()
{
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
			FExecuteAction::CreateLambda([]()
			{
				if (!ensure(GEditor))
				{
					return;
				}

				TOptional<FPlayInEditorSessionInfo> PIEInfo = GEditor->GetPlayInEditorSessionInfo();

				const UFlecsEditorDeveloperSettings* FlecsEditorDeveloperSettings = GetDefault<UFlecsEditorDeveloperSettings>();

				if UNLIKELY_IF(!ensureMsgf(IsValid(FlecsEditorDeveloperSettings),
					TEXT("Failed to get Flecs Editor Developer Settings.")))
				{
					return;
				}

				if (!PIEInfo.IsSet())
				{
					UE_LOG(LogFlecsEditor, Log, TEXT("No PIE session info found"));

					const FString TargetUrl = FlecsEditorDeveloperSettings->GetFlecsExplorerURL().ToURLString();

					// TEXT("Failed to launch Flecs Explorer URL. Explorer Instance 0")

					FPlatformProcess::LaunchURL(*TargetUrl, nullptr, nullptr);
					return;
				}

				for (int32 Index = 0; Index < PIEInfo->PIEInstanceCount; ++Index)
				{
					FString TargetUrl = FlecsEditorDeveloperSettings->GetFlecsExplorerURL().ToURLString(Index);

					FPlatformProcess::LaunchURL(*TargetUrl, nullptr, nullptr);
				}
			})
		),
		INVTEXT("Open Flecs Explorer (for each PIE instance)"),
		INVTEXT("Open Flecs Explorer (for each PIE instance)"),
		FSlateIcon("UnrealFlecsEditorStyle", "UnrealFlecs.FlecsEditor.FlecsLogo")
	));
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