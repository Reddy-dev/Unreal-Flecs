// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include <generator>

#include "Templates/SubclassOf.h"

#include "SolidMacros/Macros.h"

#include "FlecsObjectRegistrationProviderBase.generated.h"

UCLASS(Abstract, NotBlueprintable)
class UNREALFLECS_API UFlecsObjectRegistrationProviderBase : public UObject
{
	GENERATED_BODY()

public:
	virtual std::generator<TSubclassOf<UObject>> GetClassesToRegister(const bool bShouldCallAutoRegister = true) const 
		PURE_VIRTUAL(UFlecsObjectRegistrationProviderBase::GetClassesToRegister, co_return; );  
	
	static NO_DISCARD std::generator<TSubclassOf<UFlecsObjectRegistrationProviderBase>> GetAllProviders();
	static std::generator<const UFlecsObjectRegistrationProviderBase*> IterateProviders();
	
}; // class UFlecsObjectRegistrationProviderBase
