// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SequentialID.h"

#include "FlecsNetId.generated.h"

USTRUCT()
struct UNREALFLECS_API FFlecsNetId : public FSequentialIDBase
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsNetId() = default;
	
	FORCEINLINE explicit FFlecsNetId(const uint32 InID)
		: FSequentialIDBase(InID)
	{
	}
	
}; // struct FFlecsNetId


