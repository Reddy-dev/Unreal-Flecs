// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Subsystems/EngineSubsystem.h"

#include "FlecsObjectRegistrationProviderBase.generated.h"

UCLASS(Abstract, NotBlueprintable)
class UNREALFLECS_API UFlecsObjectRegistrationProviderBase : public UObject
{
	GENERATED_BODY()

public:
	virtual TArray<TSubclassOf<UObject>> GetClassesToRegister() const 
		PURE_VIRTUAL(UFlecsObjectRegistrationProviderBase::GetClassesToRegister, return TArray<TSubclassOf<UObject>>(););
	
}; // class UFlecsObjectRegistrationProviderBase
