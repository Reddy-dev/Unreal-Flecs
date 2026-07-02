// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SequentialID.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsNetworkId.generated.h"


USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsNetworkId : public FSequentialIDBase
{
	GENERATED_BODY()
	
public:
	NO_DISCARD FORCEINLINE friend uint32 GetTypeHash(const FFlecsNetworkId& InId)
	{
		return GetTypeHash(InId.GetValue());
	}

	FORCEINLINE FFlecsNetworkId() = default;
	FORCEINLINE explicit FFlecsNetworkId(const uint32 InId) : FSequentialIDBase(InId) {}
	
	
}; // struct FFlecsNetworkId

template <>
struct TFlecsComponentTraits<FFlecsNetworkId> : public TFlecsComponentTraitsBase<FFlecsNetworkId>
{
}; // struct TFlecsComponentTraits<FFlecsNetworkId>
