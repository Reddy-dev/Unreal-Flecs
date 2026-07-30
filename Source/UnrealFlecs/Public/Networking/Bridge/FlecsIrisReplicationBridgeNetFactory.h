// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Net/Iris/ReplicationSystem/NetRootObjectFactory.h"

#include "FlecsIrisReplicationBridgeNetFactory.generated.h"

/**
 * Creates the remote Iris replication bridge and binds it to the receiving
 * world's Flecs network subsystem before initial state is applied.
 */
UCLASS(Transient)
class UNREALFLECS_API UFlecsIrisReplicationBridgeNetFactory : public UNetRootObjectFactory
{
	GENERATED_BODY()

public:
	static FName GetFactoryName();

	virtual void PostInstantiation(const FPostInstantiationContext& Context) override;

protected:
	virtual void DetachedFromReplication(
		const FDetachContext& Context,
		const TOptional<FSubObjectDetachContext>& SubObjectContext) override;

}; // class UFlecsIrisReplicationBridgeNetFactory
