// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include <generator>


#include "Templates/SubclassOf.h"

#include "FlecsObjectRegistrationProviderBase.h"

#include "FlecsNativeClassObjectRegistrationProvider.generated.h"

UCLASS()
class UNREALFLECS_API UFlecsNativeClassObjectRegistrationProvider final : public UFlecsObjectRegistrationProviderBase
{
	GENERATED_BODY()

public:
	virtual std::generator<TSubclassOf<UObject>> GetClassesToRegister(const bool bShouldCallAutoRegister = true) const override;
	
}; // class UFlecsClassObjectRegistrationProvider
