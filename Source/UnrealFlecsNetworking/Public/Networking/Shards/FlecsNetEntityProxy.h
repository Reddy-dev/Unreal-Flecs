// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Networking/FlecsNetworkId.h"
#include "FlecsNetShardBase.h"
#include "Networking/Layout/FlecsReplicationSnapshot.h"

#include "FlecsNetEntityProxy.generated.h"

/** Individually replicated shard storage for one Flecs entity. */
UCLASS()
class UNREALFLECSNETWORKING_API UFlecsNetEntityProxy : public UFlecsNetShardBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void ConfigureObjectSettings(OUT UE::Net::FRootObjectSettings& OutSettings) const override;

	virtual NO_DISCARD bool CanAcceptNetEntity(const FFlecsNetworkId& InNetworkId,
		const FFlecsEntityReplicationSnapshot&) const override;
	virtual void PublishNetEntity(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot) override;
	virtual void RemoveNetEntity(const FFlecsNetworkId& InNetworkId) override;
	virtual bool IsEmpty() const override;

	void HandleReplicationDetached();

	UPROPERTY(ReplicatedUsing = OnRep_NetworkId)
	FFlecsNetworkId NetworkId;

	UFUNCTION()
	void OnRep_NetworkId();

	UPROPERTY(ReplicatedUsing = OnRep_Snapshot)
	FFlecsEntityReplicationSnapshot Snapshot;

	UFUNCTION()
	void OnRep_Snapshot();

private:

	bool bContainsEntity = false;

}; // class UFlecsNetEntityProxy
