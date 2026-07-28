// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectFactory.h"

#include "FlecsLayoutReplicator.generated.h"

/**
 * 
 */
UCLASS()
class UNREALFLECS_API UFlecsLayoutReplicator : public UObject, public INetRootObjectFactoryExtension
{
	GENERATED_BODY()

public:
	UFlecsLayoutReplicator();
	
	virtual bool IsSupportedForNetworking() const override
	{
		return true;
	}
	
	
	
}; // class UFlecsLayoutReplicator
