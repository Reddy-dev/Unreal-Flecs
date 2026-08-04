// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Net/Iris/ReplicationSystem/NetRootObjectFactory.h"

#include "FlecsNetEntityTableNetFactory.generated.h"

/** Binds dynamically-created entity tables to the receiving Flecs world. */
UCLASS(Transient)
class UNREALFLECSNETWORKING_API UFlecsNetEntityTableNetFactory : public UNetRootObjectFactory
{
	GENERATED_BODY()

public:
	static FName GetFactoryName();

	virtual void PostInstantiation(const FPostInstantiationContext& Context) override;

protected:
	virtual void DetachedFromReplication(
		const FDetachContext& Context,
		const TOptional<FSubObjectDetachContext>& SubObjectContext) override;

}; // class UFlecsNetEntityTableNetFactory
