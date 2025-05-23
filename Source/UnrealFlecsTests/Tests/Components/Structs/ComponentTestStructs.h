﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ComponentTestStructs.generated.h"

struct FTestComponent_RegisterComponentTest
{
	int32 Value;
}; // struct FTestComponent

USTRUCT()
struct FUStructTestComponent_RegisterComponentTest
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value;
}; // struct FUStructTestComponent

USTRUCT()
struct FUStructTestComponent_Inherited : public FUStructTestComponent_RegisterComponentTest
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value2;
}; // struct FUStructTestComponent_Inherited

USTRUCT()
struct FUStructTestComponent_TagUSTRUCT
{
	GENERATED_BODY()
}; // struct FUStructTestComponent_TagUSTRUCT

USTRUCT()
struct FUStructTestComponent_NonTagUSTRUCT
{
	GENERATED_BODY()

	UPROPERTY()
	bool bTest = false;
}; // struct FUStructTestComponent_NonTagUSTRUCT



