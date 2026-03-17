// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "General/FlecsModuleSettings.h"

#include "General/FlecsGameplayDeveloperSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsModuleSettings)

// Taken from Mass
void UFlecsModuleSettings::PostInitProperties()
{
	Super::PostInitProperties();
	
	if (HasAnyFlags(RF_ClassDefaultObject) && !GetClass()->HasAnyClassFlags(CLASS_Abstract))
	{
		// register with UFlecsGameplayDeveloperSettings
		GetMutableDefault<UFlecsGameplayDeveloperSettings>()->RegisterModuleSettings(this);
	}
}
