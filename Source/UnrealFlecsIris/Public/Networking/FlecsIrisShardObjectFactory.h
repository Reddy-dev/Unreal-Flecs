// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectFactory.h"

#include "FlecsIrisShardObjectFactory.generated.h"

/**
 * Iris factory that binds received aggregate Flecs shards to the destination
 * UWorld and notifies the core when a shard is detached.
 */
UCLASS(Transient)
class UNREALFLECSIRIS_API UFlecsIrisShardObjectFactory : public UNetRootObjectFactory
{
	GENERATED_BODY()

public:
	/** Stable factory name referenced by authority-side root-object settings. */
	static FName GetFactoryName();

protected:
	virtual void PostInit(const FPostInitContext& Context) override;
	virtual void DetachedFromReplication(const FDetachContext& Context,
		const TOptional<FSubObjectDetachContext>& SubObjectContext) override;
};
