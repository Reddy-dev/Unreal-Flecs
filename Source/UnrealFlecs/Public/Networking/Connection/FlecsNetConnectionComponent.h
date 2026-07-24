// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Net/Core/Connection/ConnectionHandle.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsReplicationConnectionId.h"

#include "FlecsNetConnectionComponent.generated.h"

USTRUCT()
struct UNREALFLECS_API FFlecsNetConnectionComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FFlecsReplicationConnectionId ConnectionId;
	
	UE::Net::FConnectionHandle ConnectionHandle;
	
}; // struct FFlecsNetConnectionComponent

template <>
struct TFlecsComponentTraits<FFlecsNetConnectionComponent> : public TFlecsComponentTraitsBase<FFlecsNetConnectionComponent>
{
	
}; // struct TFlecsComponentTraits<FFlecsNetConnectionComponent>
