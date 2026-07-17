// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "Stats/Stats.h"

DECLARE_STATS_GROUP(TEXT("Flecs Networking"), STATGROUP_FlecsNetworking, STATCAT_Advanced);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Full update bytes"), STAT_FlecsReplicationFullBytes,
	STATGROUP_FlecsNetworking, UNREALFLECS_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Delta update bytes"), STAT_FlecsReplicationDeltaBytes,
	STATGROUP_FlecsNetworking, UNREALFLECS_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Quantized bytes"), STAT_FlecsReplicationQuantizedBytes,
	STATGROUP_FlecsNetworking, UNREALFLECS_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Pending keys"), STAT_FlecsReplicationPendingKeys,
	STATGROUP_FlecsNetworking, UNREALFLECS_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Deferred keys"), STAT_FlecsReplicationDeferredKeys,
	STATGROUP_FlecsNetworking, UNREALFLECS_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Maximum starvation ms"), STAT_FlecsReplicationStarvationAgeMs,
	STATGROUP_FlecsNetworking, UNREALFLECS_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Chunks"), STAT_FlecsReplicationChunks,
	STATGROUP_FlecsNetworking, UNREALFLECS_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Active entities"), STAT_FlecsReplicationActiveEntities,
	STATGROUP_FlecsNetworking, UNREALFLECS_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Dormant entities"), STAT_FlecsReplicationDormantEntities,
	STATGROUP_FlecsNetworking, UNREALFLECS_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Logical routes"), STAT_FlecsReplicationRoutes,
	STATGROUP_FlecsNetworking, UNREALFLECS_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Route pages"), STAT_FlecsReplicationPages,
	STATGROUP_FlecsNetworking, UNREALFLECS_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Migrations"), STAT_FlecsReplicationMigrations,
	STATGROUP_FlecsNetworking, UNREALFLECS_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Filter allowed"), STAT_FlecsReplicationFilterAllowed,
	STATGROUP_FlecsNetworking, UNREALFLECS_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Filter denied"), STAT_FlecsReplicationFilterDenied,
	STATGROUP_FlecsNetworking, UNREALFLECS_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Reclaimed layouts"), STAT_FlecsReplicationReclaimedLayouts,
	STATGROUP_FlecsNetworking, UNREALFLECS_API);
