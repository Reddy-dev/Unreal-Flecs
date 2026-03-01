// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "General/FlecsNativeClassObjectRegistrationProvider.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "General/FlecsObjectRegistrationInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNativeClassObjectRegistrationProvider)

TArray<TSubclassOf<UObject>> UFlecsNativeClassObjectRegistrationProvider::GetClassesToRegister() const
{
	TArray<TSubclassOf<UObject>> ClassesToRegister;
	
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
			
			if (!RegistrationInterface->ShouldAutoRegisterFromCDO())
			{
				continue;
			}
				
			ClassesToRegister.Add(Class);
		}
	}
	
	return ClassesToRegister;
}
