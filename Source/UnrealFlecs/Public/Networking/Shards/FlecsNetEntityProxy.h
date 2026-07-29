// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Networking/FlecsNetworkId.h"
#include "FlecsNetShardBase.h"
#include "Networking/Layout/FlecsReplicationSnapshot.h"

#include "FlecsNetEntityProxy.generated.h"

/** Individually replicated shard storage for one Flecs entity. */
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

}; // class UFlecsNetEntityProxy
