// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Systems/FlecsSystemObject.h"

#include "FlecsGatherViewersSystems.generated.h"

UCLASS()
class UNREALFLECS_API UFlecsGatherViewersSystem : public UFlecsSystemObject
{
	GENERATED_BODY()
	
public:
	UFlecsGatherViewersSystem();
	
	virtual void BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, TFlecsSystemBuilder<>& InBuilder) const override;
	virtual void RunEachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, flecs::iter& InIterator) override;
	
private:
	TTypedFlecsQuery<flecs::pair<>> ViewerQuery;
}; // class UFlecsGatherViewersSystem