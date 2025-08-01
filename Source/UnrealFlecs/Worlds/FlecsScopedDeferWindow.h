﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Types/SolidNotNull.h"

class UFlecsWorld;

struct UNREALFLECS_API FFlecsScopedDeferWindow
{
public:
	explicit FFlecsScopedDeferWindow(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld);
	~FFlecsScopedDeferWindow();

	FFlecsScopedDeferWindow(const FFlecsScopedDeferWindow&) = delete;
	FFlecsScopedDeferWindow& operator=(const FFlecsScopedDeferWindow&) = delete;

	FORCEINLINE FFlecsScopedDeferWindow(FFlecsScopedDeferWindow&& Other)
		: FlecsWorld(MoveTemp(Other.FlecsWorld))
	{
		Other.FlecsWorld.Reset();
	}

	FORCEINLINE FFlecsScopedDeferWindow& operator=(FFlecsScopedDeferWindow&& Other)
	{
		if (this != &Other)
		{
			FlecsWorld = MoveTemp(Other.FlecsWorld);
			Other.FlecsWorld.Reset();
		}
		
		return *this;
	}
	
private:
	TWeakObjectPtr<const UFlecsWorld> FlecsWorld;
	
}; // struct FFlecsScopedDeferWindow


