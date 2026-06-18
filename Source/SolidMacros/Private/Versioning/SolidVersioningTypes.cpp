// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Versioning/SolidVersioningTypes.h"

#include "Logging/StructuredLog.h"

bool Solid::FPropertyMatchesCDO(const TSolidNotNull<const UObject*> Object,
                                const TSolidNotNull<const FProperty*> Property)
{
	solid_check(Object->GetClass()->IsChildOf(Property->GetOwnerClass()));
		
	const void* ObjectValuePtr = Property->ContainerPtrToValuePtr<void>(Object);
	const void* CDOValuePtr = Property->ContainerPtrToValuePtr<void>(Object->GetClass()->GetDefaultObject());
		
	return Property->Identical(ObjectValuePtr, CDOValuePtr);
}

bool Solid::PropertyMatchesCDO(const TSolidNotNull<const UObject*> Object, const FName PropertyName)
{
	if LIKELY_IF(const FProperty* Property = Object->GetClass()->FindPropertyByName(PropertyName))
	{
		return FPropertyMatchesCDO(Object, Property);
	}
	else
	{
		UE_LOGFMT(LogTemp, Error,
			"PropertyMatchesCDO: Property '{0}' not found on class '{1}'",
			PropertyName.ToString(), Object->GetClass()->GetName());
	}
		
	return false;
}
