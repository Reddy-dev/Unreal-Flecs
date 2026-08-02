// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsNetShardBase.h"
#include "FlecsNetEntityPageArray.h"

#include "FlecsNetEntityPage.generated.h"

/** Batched shard storage for multiple replicated Flecs entities. */
UCLASS()
class UNREALFLECSNETWORKING_API UFlecsNetEntityPage : public UFlecsNetShardBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	FFlecsNetEntityPageArray EntityPage;

	UFUNCTION()
	void OnRep_EntityPage();

}; // class UFlecsNetEntityPage
