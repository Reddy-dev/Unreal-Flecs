// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsReplicatedTrait.generated.h"

/**
 * @brief Indicates that the Replicated entity contains data when replicated over the network.
 * implies having a FFlecsReplicatedEntityComponent as well.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicatedTrait
{
	GENERATED_BODY()
}; // struct FFlecsReplicatedTrait
