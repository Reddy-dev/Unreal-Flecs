// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "FlecsObserverFlags.generated.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EFlecsObserverFlags : uint32
{
	None = 0,
	
	/* Same as query*/
	MatchPrefab = EcsObserverMatchPrefab, // (1u << 1u) 
	
	/* Same as query*/
	MatchDisabled = EcsObserverMatchDisabled, // (1u << 2u)
	
	/* Does observer have multiple terms */
	IsMulti = EcsObserverIsMulti UMETA(Hidden), // (1u << 3u)
	
	/* Is observer a monitor */
	IsMonitor = EcsObserverIsMonitor, // (1u << 4u)
	
	/* Is observer entity disabled */
	IsDisabled = EcsObserverIsDisabled UMETA(Hidden), // (1u << 5u)
	
	/* Is module parent of observer disabled  */
	IsParentDisabled = EcsObserverIsParentDisabled UMETA(Hidden), // (1u << 6u)
	
	/* Don't evaluate query for multi-component observer */
	BypassQuery = EcsObserverBypassQuery, // (1u << 7u)
	
	/* Yield matching entities when creating observer */
	YieldOnCreate = EcsObserverYieldOnCreate, // (1u << 8u)
	
	/* Yield matching entities when deleting observer */
	YieldOnDelete = EcsObserverYieldOnDelete, // (1u << 9u)
	
	/* Observer keeps component alive (same value as EcsTermKeepAlive) */
	KeepAlive = EcsObserverKeepAlive, // (1u << 10u)
	
}; // enum class EFlecsObserverFlags
ENUM_CLASS_FLAGS(EFlecsObserverFlags)
