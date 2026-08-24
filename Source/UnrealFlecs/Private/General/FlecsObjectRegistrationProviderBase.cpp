// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "General/FlecsObjectRegistrationProviderBase.h"

#include "Templates/Casts.h"
#include "UObject/UObjectIterator.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsObjectRegistrationProviderBase)

std::generator<TSubclassOf<UFlecsObjectRegistrationProviderBase>> UFlecsObjectRegistrationProviderBase::GetAllProviders()
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
		
		co_yield Class;
	}
}

std::generator<const UFlecsObjectRegistrationProviderBase*> UFlecsObjectRegistrationProviderBase::IterateProviders()
{
	for (const TSubclassOf<UFlecsObjectRegistrationProviderBase>& SubclassType : GetAllProviders())
	{
		const UFlecsObjectRegistrationProviderBase* ProviderCDO
			= CastChecked<UFlecsObjectRegistrationProviderBase>(SubclassType->GetDefaultObject());
		
		co_yield ProviderCDO;
	}
}
