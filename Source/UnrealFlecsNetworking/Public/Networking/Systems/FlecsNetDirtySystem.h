// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once


#include "Systems/FlecsSystemObject.h"

#include "FlecsNetDirtySystem.generated.h"

/**
 * 
 */
UCLASS()
class UNREALFLECSNETWORKING_API UFlecsNetDirtySystem : public UFlecsSystemObject
{
	GENERATED_BODY()

public:
	UFlecsNetDirtySystem();
	
	virtual void BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, TFlecsSystemBuilder<>& InBuilder) const override;
	virtual void EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, flecs::iter& InIterator, const FFlecsId InIndex) override;
	
}; // class UFlecsNetDirtySystem
