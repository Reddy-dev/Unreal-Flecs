// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once


#include "FlecsReplicatedValue.generated.h"

/** Serialized payload for one payload-bearing layout/dont-fragment key. */
USTRUCT()
struct UNREALFLECSNETWORKING_API FFlecsReplicatedValue
{
	GENERATED_BODY()

	UPROPERTY()
	uint16 KeyIndex = 0;

	UPROPERTY()
	TArray<uint8> Bytes;
	
}; // struct FFlecsReplicatedValue

