// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"

// @TODO: Meant to only be used internally
struct FFlecsQueryBuilderView final : flecs::query_builder_i<FFlecsQueryBuilderView>
{
	using Super = flecs::query_builder_i<FFlecsQueryBuilderView>;

	FFlecsQueryBuilderView(flecs::world_t* InWorld, const ecs_query_desc_t* InQueryDesc, int32_t term_index = 0)
		: Super(const_cast<ecs_query_desc_t*>(InQueryDesc), term_index)
		, World(InWorld)
	{
	}

protected:
	virtual flecs::world_t* world_v() override
	{
		return World;
	}

private:
	flecs::world_t* World = nullptr;
	
}; // struct FFlecsQueryBuilderView

template <typename TDesc, ecs_query_desc_t TDesc::* QueryMember>
NO_DISCARD FORCEINLINE FFlecsQueryBuilderView MakeQueryBuilderView_Internal(flecs::world_t* InWorld, const TDesc& InDesc)
{
	return FFlecsQueryBuilderView(InWorld, &(InDesc.*QueryMember));
}

template <typename ...TArgs>
NO_DISCARD FORCEINLINE FFlecsQueryBuilderView MakeQueryBuilderView_Internal(flecs::query_builder<TArgs...>& InQueryBuilder)
{
	return FFlecsQueryBuilderView(InQueryBuilder._internal_world_v(), InQueryBuilder._internal_get_query_desc());
}
