// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsReplicationLayoutId.h"
#include "Networking/FlecsReplicationKey.h"

#include "FlecsReplicationLayoutDefinition.generated.h"

/** Immutable structural definition shared by all snapshots of one Flecs table. */
USTRUCT()
struct UNREALFLECS_API FFlecsReplicationLayoutDefinition
{
	GENERATED_BODY()

	UPROPERTY()
	FFlecsReplicationLayoutId LayoutId;

	UPROPERTY()
	TArray<FFlecsReplicationKey> Keys;
}; // struct FFlecsReplicationLayoutDefinition