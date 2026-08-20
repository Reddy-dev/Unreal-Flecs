// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Systems/FlecsSystemObject.h"

#include "FlecsReplicationQueueSystem.generated.h"

/** Applies the subsystem-owned deferred replication queue during the Flecs frame. */
UCLASS()
class UNREALFLECSNETWORKING_API UFlecsReplicationQueueSystem final : public UFlecsSystemObject
{
	GENERATED_BODY()

public:
	UFlecsReplicationQueueSystem();

	virtual void BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld,
		TFlecsSystemBuilder<>& InBuilder) const override;
	virtual void RunEachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
		flecs::iter& InIterator) override;

}; // class UFlecsReplicationQueueSystem
