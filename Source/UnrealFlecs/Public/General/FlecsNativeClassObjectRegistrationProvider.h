// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsObjectRegistrationProviderBase.h"

#include "FlecsNativeClassObjectRegistrationProvider.generated.h"

UCLASS()
class UNREALFLECS_API UFlecsNativeClassObjectRegistrationProvider final : public UFlecsObjectRegistrationProviderBase
{
	GENERATED_BODY()

public:
	virtual TArray<TSubclassOf<UObject>> GetClassesToRegister() const override;
	
}; // class UFlecsClassObjectRegistrationProvider
