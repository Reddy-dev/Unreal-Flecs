// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"

#include "Layout/FlecsReplicationLayoutDefinition.h"
#include "Layout/FlecsReplicationSnapshot.h"

#include "FlecsReplicationBridgeBase.generated.h"

class UFlecsNetworkWorldSubsystem;

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
	
	virtual void InitializeBridge() PURE_VIRTUAL(UFlecsReplicationBridgeBase::InitializeBridge, );
	virtual void DeinitializeBridge() PURE_VIRTUAL(UFlecsReplicationBridgeBase::DeinitializeBridge, );
	
	// Override PublishEntityLayout, you dont need to override ReceiveEntityLayout unless you want to do something special when receiving a layout.
	virtual void PublishEntityLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition);
	virtual void ReceiveEntityLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition);
	
	// Called on both Client and Server when a new layout is published. This is called after the layout has been received and processed.
	virtual void OnEntityLayoutPublished(const FFlecsReplicationLayoutDefinition& InLayoutDefinition) {}

	/* *****IMPORTANT NOTE*****
	 * @TODO: in the future these will be reserved for creating new entities, rather than component changes, updates, AND 
	 * NEW ENTITY CREATION. For now, we will use this for both, but in the future we will need to separate these two concepts.
	 **/
	virtual void PublishNetEntity(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot)
		PURE_VIRTUAL(UFlecsReplicationBridgeBase::PublishNetEntity, );
	virtual void ReceiveNetEntity(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot)
		PURE_VIRTUAL(UFlecsReplicationBridgeBase::ReceiveNetEntity, );
	
	virtual void HandleProtocolError(const FString& InErrorMessage);
	
	
	
protected:
	NO_DISCARD TSolidNotNull<UFlecsNetworkWorldSubsystem*> GetNetworkWorldSubsystem() const;
	
	NO_DISCARD bool HasAuthority() const;
	
	// @TODO
	/*UPROPERTY(Transient)
	TWeakObjectPtr<UFlecsNetworkWorldSubsystem> NetworkWorldSubsystem;*/
	
}; // class UFlecsReplicationBridgeBase
