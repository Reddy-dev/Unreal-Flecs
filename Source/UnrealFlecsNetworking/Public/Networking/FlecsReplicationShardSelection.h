// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once


#include "Templates/SubclassOf.h"

#include "Networking/Shards/FlecsNetShardBase.h"

/** Result returned by a registered shard selector. */
struct FFlecsReplicationShardSelection
{
	TSubclassOf<UFlecsNetShardBase> ShardClass;
	FName ShardGroupKey = NAME_None;

	NO_DISCARD bool operator==(const FFlecsReplicationShardSelection& Other) const
	{
		return ShardClass == Other.ShardClass && ShardGroupKey == Other.ShardGroupKey;
	}

	NO_DISCARD bool operator!=(const FFlecsReplicationShardSelection& Other) const
	{
		return !(*this == Other);
	}

}; // struct FFlecsReplicationShardSelection
