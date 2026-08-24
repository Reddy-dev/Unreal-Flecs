// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "General/FlecsObjectRegistrationProviderBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsObjectRegistrationProviderBase)

// @TODO: use coroutines?
TArray<TSubclassOf<UFlecsObjectRegistrationProviderBase>> UFlecsObjectRegistrationProviderBase::GetAllProviders()
{
	TArray<TSubclassOf<UFlecsObjectRegistrationProviderBase>> RegisteredObjectClasses;
	
	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;

		if UNLIKELY_IF(!IsValid(Class))
		{
			continue;
		}

		if (!Class->IsChildOf(UFlecsObjectRegistrationProviderBase::StaticClass()))
		{
			continue;
		}

		if (Class == UFlecsObjectRegistrationProviderBase::StaticClass())
		{
			continue;
		}

		if (Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
		{
			continue;
		}
		
		RegisteredObjectClasses.AddUnique(Class);
	}

	return RegisteredObjectClasses;
}

// @TODO: use coroutines?
void UFlecsObjectRegistrationProviderBase::IterateProviders(TFunctionRef<void(const UFlecsObjectRegistrationProviderBase*)> Callback)
{
	for (TObjectIterator<UClass> It; It; ++It)
	{
		const UClass* Class = *It;

		if UNLIKELY_IF(!IsValid(Class))
		{
			continue;
		}

		if (!Class->IsChildOf(UFlecsObjectRegistrationProviderBase::StaticClass()))
		{
			continue;
		}

		if (Class == UFlecsObjectRegistrationProviderBase::StaticClass())
		{
			continue;
		}

		if (Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
		{
			continue;
		}
		
		const UFlecsObjectRegistrationProviderBase* ProviderCDO = CastChecked<UFlecsObjectRegistrationProviderBase>(Class->GetDefaultObject());
		Callback(ProviderCDO);
	}
}
