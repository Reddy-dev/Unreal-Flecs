// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsReplicatedValue.generated.h"

/** Serialized payload for one payload-bearing layout/dont-fragment key. */
USTRUCT()
struct UNREALFLECS_API FFlecsReplicatedValue
{
	GENERATED_BODY()

	UPROPERTY()
	uint16 KeyIndex = 0;
	
	UPROPERTY()
	bool bDontFragment = false;

	UPROPERTY()
	TArray<uint8> Bytes;
	
}; // struct FFlecsReplicatedValue


