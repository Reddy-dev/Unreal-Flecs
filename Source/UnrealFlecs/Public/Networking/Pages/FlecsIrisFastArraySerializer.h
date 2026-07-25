// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Iris/ReplicationState/IrisFastArraySerializer.h"

#include "Networking/FlecsNetworkId.h"
#include "Networking/Layout/FlecsReplicationSnapshot.h"

#include "FlecsIrisFastArraySerializer.generated.h"

class UFlecsNetEntityPageBase;

USTRUCT()
struct UNREALFLECS_API FFlecsNetEntityPageItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FFlecsNetworkId NetworkId;
	
	UPROPERTY()
	FFlecsEntityReplicationSnapshot Snapshot;
	
}; // struct FFlecsNetEntityPageItem

USTRUCT()
struct UNREALFLECS_API FFlecsNetEntityPageArray : public FIrisFastArraySerializer
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY()
	TArray<FFlecsNetEntityPageItem> Items;
	
	UPROPERTY()
	TWeakObjectPtr<UFlecsNetEntityPageBase> Owner;
	
}; // struct FFlecsNetEntityPageArray