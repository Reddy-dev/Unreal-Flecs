﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ComponentTestStructs.generated.h"

struct FTestComponent_RegisterComponentTest
{
	int32 Value;
}; // struct FTestComponent

REGISTER_FLECS_COMPONENT(FTestComponent_RegisterComponentTest);

USTRUCT()
struct FUStructTestComponent_RegisterComponentTest
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value;
}; // struct FUStructTestComponent

REGISTER_FLECS_COMPONENT(FUStructTestComponent_RegisterComponentTest);

USTRUCT()
struct FUStructTestComponent_Inherited : public FUStructTestComponent_RegisterComponentTest
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value2;
}; // struct FUStructTestComponent_Inherited

REGISTER_FLECS_COMPONENT(FUStructTestComponent_Inherited);



