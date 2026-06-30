// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SequentialID.h"

#include "FlecsNetworkId.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsNetworkId : public FSequentialIDBase
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsNetworkId() = default;
	FORCEINLINE explicit FFlecsNetworkId(const uint32 InId) : FSequentialIDBase(InId) {}
}; // struct FFlecsNetworkId


