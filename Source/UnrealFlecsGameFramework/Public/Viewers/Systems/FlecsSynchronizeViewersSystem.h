// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Systems/FlecsSystemObject.h"

#include "FlecsSynchronizeViewersSystem.generated.h"

/**
 * 
 */
UCLASS()
class UNREALFLECSGAMEFRAMEWORK_API UFlecsSynchronizeViewersSystem : public UFlecsSystemObject
{
	GENERATED_BODY()

public:
	virtual void BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, TFlecsSystemBuilder<>& InBuilder) const override;
	
	
}; // class UFlecsSynchronizeViewersSystem
