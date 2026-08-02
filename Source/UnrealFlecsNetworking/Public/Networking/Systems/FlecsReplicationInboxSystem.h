// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Systems/FlecsSystemObject.h"

#include "FlecsReplicationInboxSystem.generated.h"

UCLASS()
class UNREALFLECSNETWORKING_API UFlecsReplicationInboxSystem final : public UFlecsSystemObject
{
	GENERATED_BODY()

public:
	UFlecsReplicationInboxSystem();

	virtual void BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, TFlecsSystemBuilder<>& InBuilder) const override;
	virtual void EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
		flecs::iter& InIterator, const FFlecsId InIndex) override;

	virtual EFlecsObjectRegistrationNetworkFlags GetObjectRegistrationNetworkFlags() const override;

}; // class UFlecsReplicationInboxSystem
