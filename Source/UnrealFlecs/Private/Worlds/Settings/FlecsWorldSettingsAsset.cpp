// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Worlds/Settings/FlecsWorldSettingsAsset.h"

#include "Logging/StructuredLog.h"
#include "Misc/DataValidation.h"

#include "Pipelines/FlecsDefaultMainGameLoop.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsWorldSettingsAsset)

IMPLEMENT_SOLID_ASSET_VERSION(UFlecsWorldSettingsAsset, 0x15CC5705, 0xE0534189, 0xAA7792F1, 0x9CBAF6E8, "FlecsWorldSettingsAssetVersion");

#define LOCTEXT_NAMESPACE "FlecsWorldSettingsAsset"

UFlecsWorldSettingsAsset::UFlecsWorldSettingsAsset()
{
}

UFlecsWorldSettingsAsset::UFlecsWorldSettingsAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WorldSettings.WorldName = "DefaultFlecsWorld";
}

void UFlecsWorldSettingsAsset::PostLoad()
{
	Super::PostLoad();

	IMPLEMENT_ASSET_MIGRATION_POST_LOAD(UFlecsWorldSettingsAsset);
}

#if WITH_EDITOR

EDataValidationResult UFlecsWorldSettingsAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	
	/*if (!IsValid(WorldSettings.GameLoop))
	{
		Context.AddError(FText::Format(
			LOCTEXT("InvalidGameLoop", "WorldSettings {0} has an invalid GameLoop reference."),
			FText::FromString(GetPathName())));
			
		
		Result = EDataValidationResult::Invalid;
	}*/

	if (WorldSettings.GameLoops.IsEmpty())
	{
		Context.AddError(FText::Format(
			LOCTEXT("NoGameLoops", "WorldSettings {0} has no GameLoops assigned."),
			FText::FromString(GetPathName())));
		
		Result = EDataValidationResult::Invalid;
	}
	else
	{
		bool bHasMainLoop = false;
		TArray<TObjectPtr<UObject>> MainLoops;
		
		/*for (const TObjectPtr<UObject> GameLoop : WorldSettings.GameLoops)
		{
			if (!IsValid(GameLoop))
			{
				Context.AddError(FText::Format(
					LOCTEXT("InvalidGameLoopInArray",
						"WorldSettings {0} has an invalid GameLoop reference in its GameLoops array."),
					FText::FromString(GetPathName())));
				
				Result = EDataValidationResult::Invalid;
				continue;
			}

			if (!GameLoop->Implements<UFlecsGameLoopInterface>())
			{
				Context.AddError(FText::Format(
					LOCTEXT("NonGameLoopInArray",
						"WorldSettings {0} has a GameLoop reference in its GameLoops array that does not implement the FlecsGameLoopInterface."),
					FText::FromString(GetPathName())));
				
				Result = EDataValidationResult::Invalid;
			}
			
			const TSolidNotNull<IFlecsGameLoopInterface*> GameLoopInterface = CastChecked<IFlecsGameLoopInterface>(GameLoop);

			if (GameLoopInterface->IsMainLoop())
			{
				bHasMainLoop = true;
				MainLoops.AddUnique(GameLoop);
			}

			const TArray<FGameplayTag> GameLoopTickTypeTags = GameLoopInterface->GetTickTypeTags();
			if (GameLoopTickTypeTags.IsEmpty())
			{
				Context.AddError(FText::Format(
					LOCTEXT("NoTickTypeTags",
						"WorldSettings {0} has a GameLoop {1} in its GameLoops array that has no TickTypeTags assigned."),
					FText::FromString(GetPathName()),
					FText::FromString(GameLoop->GetClass()->GetClassPathName().ToString())));
			}

			bool bHasValidTickTypeTag = false;

			for (const FGameplayTag& TickTypeTag : GameLoopTickTypeTags)
			{
				if (AssignedTickFunctionTags.Contains(TickTypeTag))
				{
					bHasValidTickTypeTag = true;
					break;
				}
			}

			if (!bHasValidTickTypeTag)
			{
				Context.AddWarning(FText::Format(
					LOCTEXT("NoValidTickTypeTag",
						"WorldSettings {0} has a GameLoop {1} in its GameLoops array that has no TickTypeTags matching any TickFunction's TickTypeTag."),
					FText::FromString(GetPathName()),
					FText::FromString(GameLoop->GetClass()->GetClassPathName().ToString())));
			}
		}*/

		if (!bHasMainLoop)
		{
			Context.AddError(FText::Format(
				LOCTEXT("NoMainLoop",
					"WorldSettings {0} has no GameLoop marked as Main Loop in its GameLoops array."),
				FText::FromString(GetPathName())));
			
			Result = EDataValidationResult::Invalid;
		}

		for (int32 Index = 0; Index < MainLoops.Num(); ++Index)
		{
			const TSolidNotNull<const UObject*> MainLoop = MainLoops[Index];
			
			if (MainLoops.Num() > 1)
			{
				Context.AddError(FText::Format(
					LOCTEXT("MultipleMainLoops",
						"WorldSettings {0} has multiple GameLoops marked as Main Loop in its GameLoops array. Found multiple instances of {1}."),
					FText::FromString(GetPathName()),
					FText::FromString(MainLoop->GetClass()->GetClassPathName().ToString())));
				
				Result = EDataValidationResult::Invalid;
			}
		}
	}
	
	return Result;
}

#endif // WITH_EDITOR

FPrimaryAssetId UFlecsWorldSettingsAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("FlecsWorld", GetFNameSafe(this));
}

#undef LOCTEXT_NAMESPACE
