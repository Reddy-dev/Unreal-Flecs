// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Observers/FlecsObserverObject.h"

#include "FlecsReplicatedComponentObservers.generated.h"

/**
 * Server-only observer that translates per-entity replication opt-in into
 * UFlecsNetworkWorldSubsystem lifecycle calls.
 *
 * It reacts to FFlecsReplicatedEntityComponent only when the entity does not
 * already carry the internal FFlecsReplicatedTrait component-type marker.
 * Adding the entity marker begins replication; removing it stops replication.
 */
UCLASS()
class UNREALFLECSNETWORKING_API UFlecsReplicatedComponentObservers : public UFlecsObserverObject
{
	GENERATED_BODY()

public:
	UFlecsReplicatedComponentObservers();
	
	virtual void BuildObserver(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, TFlecsObserverBuilder<>& InOutBuilder) const override;
	virtual void EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, flecs::iter& InIterator, const FFlecsId InIndex) override;
	
}; // class UFlecsReplicatedComponentObservers
