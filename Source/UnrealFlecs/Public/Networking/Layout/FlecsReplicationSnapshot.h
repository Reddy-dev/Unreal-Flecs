// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsReplicationLayoutId.h"
#include "Networking/FlecsReplicationKey.h"
#include "FlecsReplicatedValue.h"

#include "FlecsReplicationSnapshot.generated.h"

USTRUCT()
struct UNREALFLECS_API FFlecsEntityReplicationSnapshot
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FFlecsReplicationLayoutId LayoutId;
	
	UPROPERTY()
	TArray<FFlecsReplicationKey> DontFragmentKeys;

	// Serves both DontFragment and Layout
	UPROPERTY()
	TArray<FFlecsReplicatedValue> Values;
	
	UPROPERTY()
	uint32 StateRevision = 0;
	
}; // struct FFlecsEntityReplicationSnapshot


