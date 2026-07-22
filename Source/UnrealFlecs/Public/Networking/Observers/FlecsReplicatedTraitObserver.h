// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Observers/FlecsObserverObject.h"
#include "FlecsReplicatedTraitObserver.generated.h"

/**
 * 
 */
UCLASS()
class UNREALFLECS_API UFlecsReplicatedTraitObserver : public UFlecsObserverObject
{
	GENERATED_BODY()

public:
	UFlecsReplicatedTraitObserver();
	
	virtual void BuildObserver(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, TFlecsObserverBuilder<>& InOutBuilder) const override;
	virtual void EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, flecs::iter& InIterator, const FFlecsId InIndex) override;
}; // class UFlecsReplicatedTraitObserver
