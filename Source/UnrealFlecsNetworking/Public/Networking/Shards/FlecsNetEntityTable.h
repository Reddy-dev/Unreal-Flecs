// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsNetShardBase.h"
#include "FlecsNetEntityTableArray.h"

#include "FlecsNetEntityTable.generated.h"

/** Table-backed shard storage for replicated Flecs entities. */
UCLASS()
class UNREALFLECSNETWORKING_API UFlecsNetEntityTable : public UFlecsNetShardBase
{
	GENERATED_BODY()

public:
	virtual void PostInitProperties() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void ConfigureObjectSettings(OUT UE::Net::FRootObjectSettings& OutSettings) const override;

	virtual bool CanAcceptNetEntity(const FFlecsNetworkId& InNetworkId,
		const FFlecsEntityReplicationSnapshot&) const override;
	virtual void PublishNetEntity(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot) override;
	virtual void RemoveNetEntity(const FFlecsNetworkId& InNetworkId) override;
	virtual bool IsEmpty() const override;

	void HandleReplicationDetached();
	void HandleEntityRemoved(const FFlecsNetworkId& InNetworkId, uint32 InStateRevision);
	void HandleEntityUpdated(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot);

	UPROPERTY(Replicated)
	FFlecsNetEntityTableArray EntityTable;

	UFUNCTION()
	void OnRep_EntityTable();

}; // class UFlecsNetEntityTable
