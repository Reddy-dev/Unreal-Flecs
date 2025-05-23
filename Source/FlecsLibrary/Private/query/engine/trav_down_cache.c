/**
 * @file query/engine/trav_down_cache.c
 * @brief Compile query term.
 */

#include "../../private_api.h"

static
void flecs_trav_entity_down_isa(
    ecs_world_t *world,
    ecs_allocator_t *a,
    ecs_trav_up_cache_t *cache,
    ecs_trav_down_t *dst,
    ecs_entity_t trav,
    ecs_entity_t entity,
    ecs_component_record_t *cr_with,
    bool self,
    bool empty);

static
ecs_trav_down_t* flecs_trav_entity_down(
    ecs_world_t *world,
    ecs_allocator_t *a,
    ecs_trav_up_cache_t *cache,
    ecs_trav_down_t *dst,
    ecs_entity_t trav,
    ecs_component_record_t *cr_trav,
    ecs_component_record_t *cr_with,
    bool self,
    bool empty);

static
ecs_trav_down_t* flecs_trav_down_ensure(
    const ecs_query_run_ctx_t *ctx,
    ecs_trav_up_cache_t *cache,
    ecs_entity_t entity)
{
    ecs_trav_down_t **trav = ecs_map_ensure_ref(
        &cache->src, ecs_trav_down_t, entity);
    if (!trav[0]) {
        trav[0] = flecs_iter_calloc_t(ctx->it, ecs_trav_down_t);
        ecs_vec_init_t(NULL, &trav[0]->elems, ecs_trav_down_elem_t, 0);
    }

    return trav[0];
}

static
ecs_trav_down_t* flecs_trav_table_down(
    ecs_world_t *world,
    ecs_allocator_t *a,
    ecs_trav_up_cache_t *cache,
    ecs_trav_down_t *dst,
    ecs_entity_t trav,
    const ecs_table_t *table,
    ecs_component_record_t *cr_with,
    bool self,
    bool empty)
{
    ecs_assert(table->id != 0, ECS_INTERNAL_ERROR, NULL);

    if (!table->_->traversable_count) {
        return dst;
    }

    ecs_assert(cr_with != NULL, ECS_INTERNAL_ERROR, NULL);

    ecs_os_perf_trace_push("flecs.trav.table_down");

    const ecs_entity_t *entities = ecs_table_entities(table);
    int32_t i, count = ecs_table_count(table);
    for (i = 0; i < count; i ++) {
        ecs_entity_t entity = entities[i];
        ecs_record_t *record = flecs_entities_get(world, entity);
        if (!record) {
            continue;
        }

        uint32_t flags = ECS_RECORD_TO_ROW_FLAGS(record->row);
        if (flags & EcsEntityIsTraversable) {
            ecs_component_record_t *cr_trav = flecs_components_get(world, 
                ecs_pair(trav, entity));
            if (!cr_trav) {
                continue;
            }

            flecs_trav_entity_down(world, a, cache, dst, 
                trav, cr_trav, cr_with, self, empty);
        }
    }

    ecs_os_perf_trace_pop("flecs.trav.table_down");
    return dst;
}

static
void flecs_trav_entity_down_isa(
    ecs_world_t *world,
    ecs_allocator_t *a,
    ecs_trav_up_cache_t *cache,
    ecs_trav_down_t *dst,
    ecs_entity_t trav,
    ecs_entity_t entity,
    ecs_component_record_t *cr_with,
    bool self,
    bool empty)
{
    if (trav == EcsIsA || !world->cr_isa_wildcard) {
        return;
    }

    ecs_component_record_t *cr_isa = flecs_components_get(
        world, ecs_pair(EcsIsA, entity));
    if (!cr_isa) {
        return;
    }

    ecs_os_perf_trace_push("flecs.trav.entity_down_isa");

    ecs_table_cache_iter_t it;
    if (flecs_table_cache_iter(&cr_isa->cache, &it)) {
        const ecs_table_record_t *tr;
        while ((tr = flecs_table_cache_next(&it, ecs_table_record_t))) {
            ecs_table_t *table = tr->hdr.table;
            if (!table->_->traversable_count) {
                continue;
            }

            if (ecs_table_has_id(world, table, cr_with->id)) {
                /* Table owns component */
                continue;
            }

            const ecs_entity_t *entities = ecs_table_entities(table);
            int32_t i, count = ecs_table_count(table);
            for (i = 0; i < count; i ++) {
                ecs_entity_t e = entities[i];
                ecs_record_t *record = flecs_entities_get(world, e);
                if (!record) {
                    continue;
                }

                uint32_t flags = ECS_RECORD_TO_ROW_FLAGS(record->row);
                if (flags & EcsEntityIsTraversable) {
                    ecs_component_record_t *cr_trav = flecs_components_get(world, 
                        ecs_pair(trav, e));
                    if (cr_trav) {
                        flecs_trav_entity_down(world, a, cache, dst, trav,
                            cr_trav, cr_with, self, empty);
                    }

                    flecs_trav_entity_down_isa(world, a, cache, dst, trav, e, 
                        cr_with, self, empty);
                }
            }
        }
    }

    ecs_os_perf_trace_pop("flecs.trav.entity_down_isa");
}

static
ecs_trav_down_t* flecs_trav_entity_down(
    ecs_world_t *world,
    ecs_allocator_t *a,
    ecs_trav_up_cache_t *cache,
    ecs_trav_down_t *dst,
    ecs_entity_t trav,
    ecs_component_record_t *cr_trav,
    ecs_component_record_t *cr_with,
    bool self,
    bool empty)
{
    ecs_assert(dst != NULL, ECS_INTERNAL_ERROR, NULL);
    ecs_assert(cr_with != NULL, ECS_INTERNAL_ERROR, NULL);
    ecs_assert(cr_trav != NULL, ECS_INTERNAL_ERROR, NULL);

    ecs_os_perf_trace_push("flecs.trav.entity_down");

    int32_t first = ecs_vec_count(&dst->elems);

    ecs_table_cache_iter_t it;
    bool result;
    if (empty) {
        result = flecs_table_cache_all_iter(&cr_trav->cache, &it);
    } else {
        result = flecs_table_cache_iter(&cr_trav->cache, &it);
    }

    if (result) {
        ecs_table_record_t *tr; 
        while ((tr = flecs_table_cache_next(&it, ecs_table_record_t))) {
            ecs_assert(tr->count == 1, ECS_INTERNAL_ERROR, NULL);
            ecs_table_t *table = tr->hdr.table;
            bool leaf = false;

            if (flecs_component_get_table(cr_with, table) != NULL) {
                if (self) {
                    continue;
                }
                leaf = true;
            }

            /* If record is not the first instance of (trav, *), don't add it
             * to the cache. */
            int32_t index = tr->index;
            if (index) {
                ecs_id_t id = table->type.array[index - 1];
                if (ECS_IS_PAIR(id) && ECS_PAIR_FIRST(id) == trav) {
                    int32_t col = ecs_search_relation(world, table, 0, 
                        cr_with->id, trav, EcsUp, NULL, NULL, &tr);
                    ecs_assert(col >= 0, ECS_INTERNAL_ERROR, NULL);

                    if (col != index) {
                        /* First relationship through which the id is 
                         * reachable is not the current one, so skip. */
                        continue;
                    }
                }
            }

            ecs_trav_down_elem_t *elem = ecs_vec_append_t(
                a, &dst->elems, ecs_trav_down_elem_t);
            elem->table = table;
            elem->leaf = leaf;
        }
    }

    /* Breadth first walk */
    int32_t t, last = ecs_vec_count(&dst->elems);
    for (t = first; t < last; t ++) {
        ecs_trav_down_elem_t *elem = ecs_vec_get_t(
            &dst->elems, ecs_trav_down_elem_t, t);
        if (!elem->leaf) {
            flecs_trav_table_down(world, a, cache, dst, trav,
                elem->table, cr_with, self, empty);
        }
    }

    ecs_os_perf_trace_pop("flecs.trav.entity_down");
    return dst;
}

ecs_trav_down_t* flecs_query_get_down_cache(
    const ecs_query_run_ctx_t *ctx,
    ecs_trav_up_cache_t *cache,
    ecs_entity_t trav,
    ecs_entity_t e,
    ecs_component_record_t *cr_with,
    bool self,
    bool empty)
{
    ecs_world_t *world = ctx->it->real_world;
    ecs_assert(cache->dir != EcsTravUp, ECS_INTERNAL_ERROR, NULL);
    
    cache->dir = EcsTravDown;

    ecs_allocator_t *a = flecs_query_get_allocator(ctx->it);
    ecs_map_init_if(&cache->src, a);

    ecs_trav_down_t *result = flecs_trav_down_ensure(ctx, cache, e);
    if (result->ready) {
        return result;
    }

    ecs_component_record_t *cr_trav = flecs_components_get(world, ecs_pair(trav, e));
    if (!cr_trav) {
        if (trav != EcsIsA) {
            flecs_trav_entity_down_isa(
                world, a, cache, result, trav, e, cr_with, self, empty);
        }
        result->ready = true;
        return result;
    }

    ecs_vec_init_t(a, &result->elems, ecs_trav_down_elem_t, 0);

    /* Cover IsA -> trav paths. If a parent inherits a component, then children
     * of that parent should find the component through up traversal. */
    if (cr_with->flags & EcsIdOnInstantiateInherit) {
        flecs_trav_entity_down_isa(
            world, a, cache, result, trav, e, cr_with, self, empty);
    }

    flecs_trav_entity_down(
        world, a, cache, result, trav, cr_trav, cr_with, self, empty);
    result->ready = true;

    return result;
}

void flecs_query_down_cache_fini(
    ecs_allocator_t *a,
    ecs_trav_up_cache_t *cache)
{
    ecs_os_perf_trace_push("flecs.query.down_cache_fini");
    
    ecs_map_iter_t it = ecs_map_iter(&cache->src);
    while (ecs_map_next(&it)) {
        ecs_trav_down_t *t = ecs_map_ptr(&it);
        ecs_vec_fini_t(a, &t->elems, ecs_trav_down_elem_t);
    }
    ecs_map_fini(&cache->src);

    ecs_os_perf_trace_pop("flecs.query.down_cache_fini");
}
