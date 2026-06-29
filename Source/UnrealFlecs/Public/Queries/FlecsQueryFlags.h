// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "FlecsQueryFlags.generated.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EFlecsQueryFlags : uint32
{
	None = 0 UMETA(Hidden),
	/**
	 * Query must match prefabs.
     * Can be combined with other query flags on the ecs_query_desc_t::flags field.
     * @ingroup queries
     */
	MatchPrefabs = EcsQueryMatchPrefab, // (1u << 1u)
	/**
	 * Query must match disabled entities.
	 * Can be combined with other query flags on the ecs_query_desc_t::flags field.
	 * @ingroup queries
	 */
	MatchDisabled = EcsQueryMatchDisabled, // (1u << 2u)
	/**
	 * Query must match empty tables.
	 * Can be combined with other query flags on the ecs_query_desc_t::flags field.
	 * @ingroup queries
	 */
	MatchEmptyTables = EcsQueryMatchEmptyTables, // (1u << 3u)
	/**
	 * Query may have unresolved entity identifiers.
     * Can be combined with other query flags on the ecs_query_desc_t::flags field.
     * @ingroup queries
     */
	AllowUnresolvedByName = EcsQueryAllowUnresolvedByName, // (1u << 6u)
	/**
	 * Query only returns whole tables (ignores toggle/member fields).
	 * Can be combined with other query flags on the ecs_query_desc_t::flags field.
	 * @ingroup queries
	 */
	TableOnly = EcsQueryTableOnly, // (1u << 7u)
	
	/** Enable change detection for query.
	 * Can be combined with other query flags on the ecs_query_desc_t::flags field.
	 * 
	 * Adding this flag makes it possible to use ecs_query_changed() and 
	 * ecs_iter_changed() with the query. Change detection requires the query to be
	 * cached. If cache_kind is left to the default value, this flag will cause it
	 * to default to EcsQueryCacheAuto.
	 * 
	 * \ingroup queries
	 */
	DetectChanges = EcsQueryDetectChanges, // (1u << 8u)
	
	/** Enable ordering for query groups.
	 * When this flag is set, groups will be iterated in ascending order, with lower
	 * group ids first and higher group ids afterwards.
	 * 
	 * This flag is enabled automatically when a query contains cascade terms.
	 * 
	 * \ingroup queries
	 */
	GroupByOrdered = EcsQueryGroupByOrdered, // (1u << 9u)
	
	/** Enable descending ordering for query groups.
	 * When this flag is set in combination with EcsQueryGroupByOrdered, groups will 
	 * be iterated in descending order, with higher group ids first and lower group 
	 * ids afterwards.
	 * 
	 * This flag is enabled automatically when a query contains cascade|desc terms.
	 * 
	 * \ingroup queries
	 */
	GroupByOrderedDescending = EcsQueryGroupByDesc, // (1u << 10u)
	
}; // enum class EFlecsQueryFlags

ENUM_CLASS_FLAGS(EFlecsQueryFlags)

