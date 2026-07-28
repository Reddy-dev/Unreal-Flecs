// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Networking/Shards/FlecsNetShardBase.h"
#include "FlecsIrisFastArraySerializer.h"

#include "FlecsNetEntityPage.generated.h"

/**
 * 
 */
UCLASS()
class UNREALFLECS_API UFlecsNetEntityPage : public UFlecsNetShardBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(Replicated)
	FFlecsNetEntityPageArray EntityPage;
	
	UFUNCTION()
	void OnRep_EntityPage();
	
}; // class UFlecsNetEntityPageBase
