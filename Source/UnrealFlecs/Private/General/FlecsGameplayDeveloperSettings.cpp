// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "General/FlecsGameplayDeveloperSettings.h"

#include "General/FlecsModuleSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsGameplayDeveloperSettings)

void UFlecsGameplayDeveloperSettings::RegisterModuleSettings(const TSolidNotNull<UFlecsModuleSettings*> InModuleSettings)
{
	solid_checkf(InModuleSettings->HasAnyFlags(RF_ClassDefaultObject), TEXT("Registered ModuleSettings need to be its class's CDO"));
	
	// we should consider a replacement in case we're hot-reloading
	FName EntryName = InModuleSettings->GetClass()->GetFName();

#if WITH_EDITOR
	
	static const FName DisplayNameMeta(TEXT("DisplayName")); 
	
	// try reading better name from meta data, available only in editor. Besides, we don't really care about this out 
	// side of editor. We could even skip populating ModuleSettings but we'll leave it as is for now.
	const FString& DisplayNameValue = InModuleSettings->GetClass()->GetMetaData(DisplayNameMeta);
	
	if (DisplayNameValue.Len())
	{
		EntryName = *DisplayNameValue;
	}
	
#endif // WITH_EDITOR

	TObjectPtr<UFlecsModuleSettings>& FoundModuleEntry = ModuleSettings.FindOrAdd(EntryName, 
		TObjectPtr<UFlecsModuleSettings>(InModuleSettings));
	FoundModuleEntry = InModuleSettings;
}
