// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"

#include "Layout/FlecsReplicationLayoutDefinition.h"
#include "Layout/FlecsReplicationSnapshot.h"
#include "Router/FlecsNetRouteId.h"

#include "FlecsReplicationBridgeBase.generated.h"

class UFlecsNetworkWorldSubsystem;
class UFlecsNetShardBase;

/**
 * 
 */
UCLASS(Abstract, BlueprintType, NotBlueprintable)
class UNREALFLECS_API UFlecsReplicationBridgeBase : public UObject
{
	GENERATED_BODY()

public:
	UFlecsReplicationBridgeBase(const FObjectInitializer& ObjectInitializer);
	virtual ~UFlecsReplicationBridgeBase() override;
	
	virtual bool IsSupportedForNetworking() const override
	{
		return true;
	}

	virtual void InitializeBridge() {}
	virtual void DeinitializeBridge() {}
	
	// Override PublishEntityLayout, you dont need to override ReceiveEntityLayout unless you want to do something special when receiving a layout.
	virtual void PublishEntityLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition)
		PURE_VIRTUAL(UFlecsReplicationBridgeBase::PublishEntityLayout, );
	virtual void ReceiveEntityLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition);
	
	// Called on both Client and Server when a new layout is published. This is called after the layout has been received and processed.
	virtual void OnEntityLayoutPublished(const FFlecsReplicationLayoutDefinition& InLayoutDefinition) {}

	/* 
	 * *****IMPORTANT NOTE*****
	 * @TODO: in the future these will be reserved for creating new entities, rather than component changes, updates, AND 
	 * NEW ENTITY CREATION. For now, we will use this for both, but in the future we will need to separate these two concepts.
	 **/
	virtual void PublishNetEntity(
		const FFlecsNetRouteId& InRouteId,
		const FFlecsNetworkId& InNetworkId,
		const FFlecsEntityReplicationSnapshot& InSnapshot)
		PURE_VIRTUAL(UFlecsReplicationBridgeBase::PublishNetEntity, );
	
	virtual void ReceiveNetEntity(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot);
	
	virtual void HandleProtocolError(const FString& InErrorMessage);

	/** Resolves the generic storage object selected by a route. */
	virtual NO_DISCARD UFlecsNetShardBase* ResolveShard(const FFlecsNetRouteId& InRouteId)
		PURE_VIRTUAL(UFlecsReplicationBridgeBase::ResolveShard, return nullptr;);

	void SetNetworkWorldSubsystem(UFlecsNetworkWorldSubsystem* InNetworkWorldSubsystem);

	NO_DISCARD bool HasAuthority() const;

protected:
	NO_DISCARD TSolidNotNull<UFlecsNetworkWorldSubsystem*> GetNetworkWorldSubsystem() const;
	
	UPROPERTY(Transient)
	TWeakObjectPtr<UFlecsNetworkWorldSubsystem> NetworkWorldSubsystem;
	
}; // class UFlecsReplicationBridgeBase
