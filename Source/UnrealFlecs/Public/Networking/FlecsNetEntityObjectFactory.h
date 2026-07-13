// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Iris/ReplicationSystem/NetObjectFactory.h"
#include "FlecsNetEntityObjectFactory.generated.h"

/**
 * 
 */
UCLASS()
class UNREALFLECS_API UFlecsNetEntityObjectFactory : public UNetObjectFactory
{
	GENERATED_BODY()

public:
	static FName GetFactoryName();
	
}; // class UFlecsNetEntityObjectFactory
