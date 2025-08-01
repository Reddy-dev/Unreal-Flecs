﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/FlecsModuleObject.h"
#include "FlecsTranslationModule.generated.h"

// @TODO: Not Implemented
UCLASS(BlueprintType, NotBlueprintable, meta = (DisplayName = "Flecs Translation Module"))
class UNREALFLECS_API UFlecsTranslationModule final : public UFlecsModuleObject
{
	GENERATED_BODY()

public:
	FORCEINLINE virtual FString GetModuleName_Implementation() const override
	{
		return "Flecs Translation Module";
	}
	
	virtual void InitializeModule(TSolidNotNull<UFlecsWorld*> InWorld, const FFlecsEntityHandle& InModuleEntity) override;
	virtual void DeinitializeModule(TSolidNotNull<UFlecsWorld*> InWorld) override;

private:

}; // class UFlecsTranslationModule
