// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Net/Iris/ReplicationSystem/NetRootObjectFactory.h"

#include "FlecsLayoutReplicatorNetFactory.generated.h"

/**
 * Creates the remote layout catalogue and binds it to the receiving world's
 * local Flecs replication bridge before initial state is applied.
 */
UCLASS(Transient)
class UNREALFLECS_API UFlecsLayoutReplicatorNetFactory : public UNetRootObjectFactory
{
	GENERATED_BODY()

public:
	static FName GetFactoryName();

	virtual void PostInstantiation(const FPostInstantiationContext& Context) override;

}; // class UFlecsLayoutReplicatorNetFactory
