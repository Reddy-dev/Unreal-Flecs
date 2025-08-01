﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"
#include "FlecsQueryFlags.generated.h"

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EFlecsQueryFlags : uint8
{
	None = 0 UMETA(Hidden),
	/**
	 * Query must match prefabs.
     * Can be combined with other query flags on the ecs_query_desc_t::flags field.
     * @ingroup queries
     */
	MatchPrefabs = EcsQueryMatchPrefab,
	/**
	 * Query must match disabled entities.
	 * Can be combined with other query flags on the ecs_query_desc_t::flags field.
	 * @ingroup queries
	 */
	MatchDisabled = EcsQueryMatchDisabled,
	/**
	 * Query must match empty tables.
	 * Can be combined with other query flags on the ecs_query_desc_t::flags field.
	 * @ingroup queries
	 */
	MatchEmptyTables = EcsQueryMatchEmptyTables,
	/**
	 * Query may have unresolved entity identifiers.
     * Can be combined with other query flags on the ecs_query_desc_t::flags field.
     * @ingroup queries
     */
	AllowUnresolvedByName = EcsQueryAllowUnresolvedByName,
	/**
	 * Query only returns whole tables (ignores toggle/member fields).
	 * Can be combined with other query flags on the ecs_query_desc_t::flags field.
	 * @ingroup queries
	 */
	TableOnly = EcsQueryTableOnly,
	
}; // enum class EFlecsQueryFlags

ENUM_CLASS_FLAGS(EFlecsQueryFlags)

