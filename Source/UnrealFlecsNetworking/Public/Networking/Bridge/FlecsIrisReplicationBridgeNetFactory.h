// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/World.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectFactory.h"

#include "FlecsIrisReplicationBridgeNetFactory.generated.h"

class UFlecsIrisReplicationBridge;

/**
 * Creates the remote Iris replication bridge and binds it to the receiving
 * world's Flecs network subsystem before initial state is applied.
 */
UCLASS(Transient)
class UNREALFLECSNETWORKING_API UFlecsIrisReplicationBridgeNetFactory : public UNetRootObjectFactory
{
	GENERATED_BODY()

public:
	static FName GetFactoryName();

	virtual void PostInstantiation(const FPostInstantiationContext& Context) override;

protected:
	virtual void DetachedFromReplication(
		const FDetachContext& Context,
		const TOptional<FSubObjectDetachContext>& SubObjectContext) override;
	virtual void OnDeinit() override;

private:
	void ResolvePendingReplicationBridge();
	void HandleWorldPreActorTick(UWorld* InWorld, ELevelTick, float);
	void StopReplicationBridgeRetry();

	UPROPERTY()
	TWeakObjectPtr<UFlecsIrisReplicationBridge> PendingReplicationBridge;
	
	FDelegateHandle WorldPreActorTickHandle;

}; // class UFlecsIrisReplicationBridgeNetFactory
