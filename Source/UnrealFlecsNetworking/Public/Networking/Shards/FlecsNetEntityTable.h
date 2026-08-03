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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	FFlecsNetEntityTableArray EntityTable;

	UFUNCTION()
	void OnRep_EntityTable();

}; // class UFlecsNetEntityTable
