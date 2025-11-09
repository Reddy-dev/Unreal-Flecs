// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsWorldSettingsAsset.h"

#include "Logging/StructuredLog.h"

#include "Logs/FlecsCategories.h"
#include "Modules/FlecsModuleInterface.h"

#include "Modules/FlecsModuleSetDataAsset.h"
#include "Pipelines/FlecsDefaultGameLoop.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsWorldSettingsAsset)

#define LOCTEXT_NAMESPACE "FlecsWorldSettingsAsset"

UFlecsWorldSettingsAsset::UFlecsWorldSettingsAsset()
{
	WorldSettings.WorldName = "DefaultFlecsWorld";
}

void UFlecsWorldSettingsAsset::PostLoad()
{
	Super::PostLoad();

#if WITH_EDITOR
	

#endif // WITH_EDITOR
}

#if WITH_EDITOR

EDataValidationResult UFlecsWorldSettingsAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	
	/*if (!IsValid(WorldSettings.GameLoop))
	{
		Context.AddError(FText::Format(
			LOCTEXT("InvalidGameLoop", "WorldSettings {PathName} has an invalid GameLoop reference."),
			FText::FromString(GetPathName())));
			
		
		Result = EDataValidationResult::Invalid;
	}*/

	if (WorldSettings.GameLoops.IsEmpty())
	{
		Context.AddError(FText::Format(
			LOCTEXT("NoGameLoops", "WorldSettings {PathName} has no GameLoops assigned."),
			FText::FromString(GetPathName())));
			
		
		Result = EDataValidationResult::Invalid;
	}
	else
	{
		for (const TObjectPtr<UObject> GameLoop : WorldSettings.GameLoops)
		{
			if (!IsValid(GameLoop))
			{
				Context.AddError(FText::Format(
					LOCTEXT("InvalidGameLoopInArray", "WorldSettings {PathName} has an invalid GameLoop reference in its GameLoops array."),
					FText::FromString(GetPathName())));
				
				Result = EDataValidationResult::Invalid;
			}
		}
	}

	TArray<TObjectPtr<UObject>> ImportedModules;
	
	ImportedModules.Append(WorldSettings.Modules);
	ImportedModules.Append(WorldSettings.EditorModules);
	ImportedModules.Append(WorldSettings.GameLoops);
	
	for (const UFlecsModuleSetDataAsset* ModuleSet : WorldSettings.ModuleSets)
	{
		if (!IsValid(ModuleSet))
		{
			Context.AddError(FText::Format(
				LOCTEXT("InvalidModuleSet", "WorldSettings {PathName} has an invalid ModuleSet reference."),
				FText::FromString(GetPathName())));
			
			Result = EDataValidationResult::Invalid;
			continue;
		}

		ImportedModules.Append(ModuleSet->Modules);
	}

	for (const UFlecsModuleSetDataAsset* ModuleSet : WorldSettings.EditorModuleSets)
	{
		if (!IsValid(ModuleSet))
		{
			Context.AddError(FText::Format(
				LOCTEXT("InvalidEditorModuleSet",
					"WorldSettings {PathName} has an invalid EditorModuleSet reference."),
				FText::FromString(GetPathName())));
			
			Result = EDataValidationResult::Invalid;
			continue;
		}

		ImportedModules.Append(ModuleSet->Modules);
	}

	const EDataValidationResult DuplicateModuleResult = CheckForDuplicateModules(Context, ImportedModules);
	Result = CombineDataValidationResults(Result, DuplicateModuleResult);

	const EDataValidationResult HardDependencyResult = CheckForHardDependencies(Context, ImportedModules);
	Result = CombineDataValidationResults(Result, HardDependencyResult);
	
	return Result;
}

EDataValidationResult UFlecsWorldSettingsAsset::CheckForDuplicateModules(FDataValidationContext& Context,
	const TArrayView<TObjectPtr<UObject>> ImportedModules) const
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	TSet<TSubclassOf<UObject>> SeenModules;
	for (const UObject* Module : ImportedModules)
	{
		if UNLIKELY_IF(!Module)
		{
			// We should never reach this point due to validation previously done in the callstack
			UE_LOGFMT(LogFlecsCore, Warning,
				"WorldSettings {PathName} has a null module reference.", GetPathName());
			continue;
		}

		TSubclassOf<UObject> ModuleClass = Module->GetClass();
		
		if (SeenModules.Contains(ModuleClass))
		{
			Context.AddError(FText::Format(
				LOCTEXT("DuplicateModule", "WorldSettings {PathName} has duplicate module of class {ModuleClass}."),
				FText::FromString(GetPathName()),
				FText::FromString(ModuleClass->GetClassPathName().ToString())));
			
			Result = EDataValidationResult::Invalid;
		}
		else
		{
			SeenModules.Add(ModuleClass);
		}
	}

	return Result;
}

EDataValidationResult UFlecsWorldSettingsAsset::CheckForHardDependencies(FDataValidationContext& Context,
	TArrayView<TObjectPtr<UObject>> ImportedModules) const
{
	EDataValidationResult Result = EDataValidationResult::Valid;
	
	for (const UObject* Module : ImportedModules)
	{
		if UNLIKELY_IF(!IsValid(Module))
		{
			Context.AddError(FText::Format(
				NSLOCTEXT("Flecs",
				"FlecsModuleSetDataAsset_InvalidModule",
				"Module Set '{0}' has an invalid module reference!"),
				FText::FromName(GetFName())));
			
			Result = EDataValidationResult::Invalid;
			continue;
		}

		const TSolidNotNull<const IFlecsModuleInterface*> ModuleInterface = CastChecked<IFlecsModuleInterface>(Module);

		const TArray<TSubclassOf<UFlecsModuleInterface>> HardDependencies = ModuleInterface->GetHardDependentModuleClasses();

		for (const TSubclassOf<UFlecsModuleInterface> HardDependencyClass : HardDependencies)
		{
			bool bFoundDependency = false;

			for (const UObject* OtherModule : ImportedModules)
			{
				if UNLIKELY_IF(!IsValid(OtherModule))
				{
					continue;
				}

				if (OtherModule->IsA(HardDependencyClass))
				{
					bFoundDependency = true;
					break;
				}
			}

			if UNLIKELY_IF(!bFoundDependency)
			{
				Context.AddError(FText::Format(
					NSLOCTEXT("Flecs",
						"FlecsModuleSetDataAsset_MissingHardDependency",
						"Module Set '{0}' is missing hard dependency '{1}' required by module '{2}'!"),
						FText::FromName(GetFName()),
						FText::FromName(HardDependencyClass->GetDefaultObjectName()),
						FText::FromName(Module->GetFName())));
				
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
