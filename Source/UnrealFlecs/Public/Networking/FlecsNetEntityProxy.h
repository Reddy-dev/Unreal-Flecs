// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsNetworkId.h"
#include "Networking/Shards/FlecsNetShardBase.h"
#include "Layout/FlecsReplicationSnapshot.h"

#include "FlecsNetEntityProxy.generated.h"

/**
 * 
 */
UCLASS()
class UNREALFLECS_API UFlecsNetEntityProxy : public UFlecsNetShardBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void PublishNetEntity(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot) override;

	UPROPERTY(ReplicatedUsing = OnRep_NetworkId)
	FFlecsNetworkId NetworkId;
	
	UFUNCTION()
	void OnRep_NetworkId();

	UPROPERTY(ReplicatedUsing = OnRep_Snapshot)
	FFlecsEntityReplicationSnapshot Snapshot;
	
	UFUNCTION()	
	void OnRep_Snapshot();

}; // class UFlecsNetEntityProxyBase
