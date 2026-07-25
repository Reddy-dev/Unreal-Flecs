// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Iris/ReplicationState/IrisFastArraySerializer.h"

#include "FlecsIrisFastArraySerializer.generated.h"

USTRUCT()
struct UNREALFLECS_API FFlecsNetEntityPageItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
}; // struct FFlecsNetEntityPageItem

USTRUCT()
struct UNREALFLECS_API FFlecsNetEntityPageArray : public FIrisFastArraySerializer
{
	GENERATED_BODY()
}; // struct FFlecsNetEntityPageArray