// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"

#include "FlecsReplicationBridgeBase.generated.h"

/**
 * 
 */
UCLASS(Abstract, BlueprintType, NotBlueprintable)
class UNREALFLECS_API UFlecsReplicationBridgeBase : public UObject
{
	GENERATED_BODY()

public:
	UFlecsReplicationBridgeBase(const FObjectInitializer& ObjectInitializer);
	
	
	
}; // class UFlecsReplicationBridgeBase
