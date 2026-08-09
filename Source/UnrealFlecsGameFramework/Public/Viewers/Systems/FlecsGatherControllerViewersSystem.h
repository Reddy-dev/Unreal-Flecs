// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Systems/FlecsSystemObject.h"

#include "FlecsGatherControllerViewersSystem.generated.h"

/**
 * 
 */
UCLASS()
class UNREALFLECSGAMEFRAMEWORK_API UFlecsGatherControllerViewersSystem : public UFlecsSystemObject
{
	GENERATED_BODY()

public:
	UFlecsGatherControllerViewersSystem(const FObjectInitializer& ObjectInitializer);
	
	virtual void BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, TFlecsSystemBuilder<>& InBuilder) const override;
	virtual void RunEachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, flecs::iter& InIterator) override;
	
	
}; // class UFlecsGatherControllerViewersSystem
