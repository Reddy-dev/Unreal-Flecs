// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "General/FlecsNativeClassObjectRegistrationProvider.h"

#include "Engine/Engine.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "General/FlecsObjectRegistrationInterface.h"
#include "UObject/UObjectIterator.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNativeClassObjectRegistrationProvider)

std::generator<TSubclassOf<UObject>> UFlecsNativeClassObjectRegistrationProvider::GetClassesToRegister(const bool bShouldCallAutoRegister) const
{
	for (TObjectIterator<UClass> It = TObjectIterator<UClass>(); It; ++It)
	{
		const TSolidNotNull<UClass*> Class = *It;
			
		if (Class->ImplementsInterface(UFlecsObjectRegistrationInterface::StaticClass()))
		{
			if (Class->HasAnyClassFlags(CLASS_Abstract))
			{
				continue;
			}
				
			if UNLIKELY_IF(Class->HasAnyClassFlags(CLASS_Deprecated | CLASS_NewerVersionExists))
			{
				continue;
			}
			
			if (!Class->HasAnyClassFlags(CLASS_Native))
			{
				continue;
			}
			
			const TSolidNotNull<const UObject*> DefaultObject = Class->GetDefaultObject();
			const TSolidNotNull<const IFlecsObjectRegistrationInterface*> RegistrationInterface 
				= CastChecked<IFlecsObjectRegistrationInterface>(DefaultObject);
			
			if (bShouldCallAutoRegister)
			{
				if (!RegistrationInterface->ShouldAutoRegisterFromCDO())
				{
					continue;
				}
			}
			
#if WITH_AUTOMATION_TESTS
			
			if (!GIsAutomationTesting)
			{
				if (RegistrationInterface->ShouldAutoRegisterOnlyForTest())
				{
					continue;
				}
			}
			
#endif // WITH_AUTOMATION_TESTS
				
			co_yield Class;
		}
	}
}
