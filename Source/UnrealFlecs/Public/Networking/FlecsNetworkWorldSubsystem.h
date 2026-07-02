// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Worlds/FlecsAbstractWorldSubsystem.h"
#include "FlecsNetworkId.h"

#include "FlecsNetworkWorldSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class UNREALFLECS_API UFlecsNetworkWorldSubsystem : public UFlecsAbstractWorldSubsystem
{
	GENERATED_BODY()

public:
	UFlecsNetworkWorldSubsystem();
	
	FFlecsNetworkId BeginReplicatingEntity(const FFlecsEntityHandle& EntityHandle);
	void StopReplicatingEntity(const FFlecsNetworkId& NetworkId);
	
private:
	
	TMap<FFlecsNetworkId, FFlecsEntityHandle> NetworkIdToEntityHandleMap;
	TMap<FFlecsEntityHandle, FFlecsNetworkId> EntityHandleToNetworkIdMap;
	
}; // class UFlecsNetworkWorldSubsystem
