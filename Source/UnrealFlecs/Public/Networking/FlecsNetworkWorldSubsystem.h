// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Worlds/FlecsAbstractWorldSubsystem.h"
#include "FlecsNetworkId.h"

#include "FlecsNetworkWorldSubsystem.generated.h"

class UFlecsNetworkingModuleSettings;
class AFlecsReplicationBridgeBase;

/**
 * 
 */
UCLASS()
class UNREALFLECS_API UFlecsNetworkWorldSubsystem : public UFlecsAbstractWorldSubsystem
{
	GENERATED_BODY()

public:
	UFlecsNetworkWorldSubsystem();
	
	virtual void OnFlecsWorldInitialized(const TSolidNotNull<UFlecsWorld*> InWorld) override;
	
	FFlecsNetworkId BeginReplicatingEntity(const FFlecsEntityHandle& EntityHandle);
	
	void StopReplicatingEntity(const FFlecsEntityHandle& EntityHandle);
	void StopReplicatingEntity(const FFlecsNetworkId& NetworkId);
	
	template <class T>
	NO_DISCARD FORCEINLINE T* GetReplicationBridge() const
	{
		return Cast<T>(FlecsReplicationBridge);
	}
	
	template <class T>
	NO_DISCARD FORCEINLINE TSolidNotNull<T*> GetReplicationBridgeChecked() const
	{
		return TSolidNotNull<T*>(CastChecked<T>(FlecsReplicationBridge));
	}
	
protected:
	NO_DISCARD bool IsNetModeServer() const;
	
	virtual void CreateReplicationBridge();
	
	NO_DISCARD TSolidNotNull<UFlecsNetworkingModuleSettings*> GetNetworkingModuleSettings() const;
	
private:
	NO_DISCARD FFlecsNetworkId GenerateNewNetworkId();
	
	UPROPERTY()
	uint32 NextNetworkId = 0;
	
	UPROPERTY()
	TMap<FFlecsNetworkId, FFlecsEntityHandle> NetworkIdToEntityHandleMap;
	
	UPROPERTY()
	TMap<FFlecsEntityHandle, FFlecsNetworkId> EntityHandleToNetworkIdMap;
	
	UPROPERTY()
	TObjectPtr<AFlecsReplicationBridgeBase> FlecsReplicationBridge;
	
	
}; // class UFlecsNetworkWorldSubsystem
