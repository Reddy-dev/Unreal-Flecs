// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsNetworkId.h"
#include "Networking/Shards/FlecsNetShardBase.h"
#include "Layout/FlecsReplicationSnapshot.h"

#include "FlecsNetEntityProxyBase.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class UNREALFLECS_API UFlecsNetEntityProxyBase : public UFlecsNetShardBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_NetworkId)
	FFlecsNetworkId NetworkId;
	
	UFUNCTION()	
	void OnRep_NetworkId();

	UPROPERTY(ReplicatedUsing = OnRep_Snapshot)
	FFlecsEntityReplicationSnapshot Snapshot;
	
	UFUNCTION()	
	void OnRep_Snapshot();

}; // class UFlecsNetEntityProxyBase
