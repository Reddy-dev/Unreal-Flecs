// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "FlecsQueryCache.generated.h"

UENUM(BlueprintType)
enum class EFlecsQueryCacheType : uint8
{
	/**< Behavior determined by query creation context */
	Default = flecs::QueryCacheDefault,
	/**< Cache query terms that are cacheable */
	Auto = flecs::QueryCacheAuto,
	/**< Require that all query terms can be cached */
	All = flecs::QueryCacheAll,
	/**< No caching */
	None = flecs::QueryCacheNone
}; // enum class EFlecsQueryCacheType

NO_DISCARD FORCEINLINE constexpr flecs::query_cache_kind_t ToFlecsQueryCache(EFlecsQueryCacheType CacheType)
{
	return static_cast<flecs::query_cache_kind_t>(CacheType);
}