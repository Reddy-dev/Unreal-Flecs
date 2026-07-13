// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Info.h"

#include "FlecsReplicationBridgeBase.generated.h"

UCLASS(Abstract, BlueprintType)
class UNREALFLECS_API AFlecsReplicationBridgeBase : public AInfo
{
	GENERATED_BODY()

public:
	AFlecsReplicationBridgeBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;


}; // AFlecsReplicationBridgeBase
