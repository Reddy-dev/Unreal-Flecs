// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsReplicationCommandBuffer.generated.h"

USTRUCT()
struct UNREALFLECS_API FFlecsReplicationCommandBuffer
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY()
	TArray<uint8> Buffer;
	
}; // struct FFlecsReplicationCommandBuffer

