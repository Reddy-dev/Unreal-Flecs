// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Net/Iris/ReplicationSystem/NetRootObjectFactory.h"

#include "FlecsNetEntityProxyNetFactory.generated.h"

/** Binds dynamically-created entity proxies to the receiving Flecs world. */
UCLASS(Transient)
class UNREALFLECSNETWORKING_API UFlecsNetEntityProxyNetFactory : public UNetRootObjectFactory
{
	GENERATED_BODY()

public:
	static FName GetFactoryName();

protected:
	virtual FInstantiateResult InstantiateReplicatedObjectFromHeader(
		const FInstantiateContext& Context,
		const UE::Net::FNetObjectCreationHeader* Header) override;

	virtual void DetachedFromReplication(
		const FDetachContext& Context,
		const TOptional<FSubObjectDetachContext>& SubObjectContext) override;
	
	virtual TOptional<FWorldInfoData> GetWorldInfo(const FWorldInfoContext& Context) const override;

}; // class UFlecsNetEntityProxyNetFactory
