// Elie Wiese-Namir © 2025. All Rights Reserved.

#if WITH_AUTOMATION_TESTS

#include "Bake/FlecsTestUtils.h"

#define MAX_SYS_COLUMNS (20)
#define MAX_ENTITIES (256)
#define MAX_INVOCATIONS (1024)

/* Multiline strings */
#define HEAD
#define LINE "\n"

typedef struct Probe {
	ecs_entity_t system;
	ecs_entity_t event;
	ecs_id_t event_id;
	int32_t offset;
	int32_t count;
	int32_t invoked;
	int32_t term_count;
	int32_t term_index;
	ecs_entity_t e[MAX_ENTITIES];
	ecs_entity_t c[MAX_INVOCATIONS][MAX_SYS_COLUMNS];
	ecs_entity_t s[MAX_INVOCATIONS][MAX_SYS_COLUMNS];
	ecs_flags32_t ref_fields;
	ecs_flags32_t up_fields;
	ecs_flags32_t row_fields;
	void *param;
} Probe;

typedef struct IterData {
	ecs_entity_t component;
	ecs_entity_t component_2;
	ecs_entity_t component_3;
	ecs_entity_t new_entities[MAX_ENTITIES];
	int32_t entity_count;
} IterData;

typedef struct Position {
	float x;
	float y;
} Position;

typedef struct Velocity {
	float x;
	float y;
} Velocity;

typedef float Mass;

typedef float Rotation;

typedef struct Color {
	float r;
	float g;
	float b;
	float a;
} Color;

typedef struct Self {
	ecs_entity_t value;
} Self;

inline void probe_system_w_ctx(
	ecs_iter_t *it,
	Probe *ctx)
{
	if (!ctx) {
		return;
	}

	ctx->param = it->param;
	ctx->system = it->system;
	ctx->event = it->event;
	ctx->event_id = it->event_id;
	ctx->offset = 0;
	ctx->term_count = it->field_count;
	ctx->term_index = it->term_index;
	ctx->ref_fields = it->ref_fields;
	ctx->up_fields = it->up_fields;
	ctx->row_fields = it->row_fields;

	int i;
	for (i = 0; i < ctx->term_count; i ++) {
		ctx->c[ctx->invoked][i] = it->ids[i];
		ctx->s[ctx->invoked][i] = ecs_field_src(it, i);

		ecs_id_t field_id = ecs_field_id(it, i);
		test_assert(field_id != 0);

		if (ecs_field_is_set(it, i)) {
			if (it->trs[i]) {
				if (it->sources[i]) {
					ecs_table_t *table = ecs_get_table(it->world, it->sources[i]);
					test_assert(it->trs[i]->hdr.table == table);
				} else {
					test_assert(it->trs[i]->hdr.table == it->table);
				}
			}
		}
	}

	for (i = 0; i < it->count; i ++) {
		if (i + ctx->count < 256) {
			ctx->e[i + ctx->count] = it->entities[i];
		} else {
			/* can't store more than that, tests shouldn't rely on
				* getting back more than 256 results */
		}
	}
	ctx->count += it->count;

	ctx->invoked ++;
}

inline void probe_iter(ecs_iter_t *it)
{
	Probe *ctx = static_cast<Probe*>(ecs_get_ctx(it->world));
	if (!ctx) {
		ctx = static_cast<Probe*>(it->ctx);
	}
	probe_system_w_ctx(it, ctx);
}

inline void probe_has_entity(Probe *probe, ecs_entity_t e)
{
	int i;
	for (i = 0; i < probe->count; i ++) {
		if (probe->e[i] == e) {
			break;
		}
	}

	test_assert(i != probe->count);
}

inline const ecs_entity_t* bulk_new_w_type(
	ecs_world_t *world, ecs_entity_t type_ent, int32_t count)
{
	const ecs_type_t *type = ecs_get_type(world, type_ent);
	test_assert(type != NULL);

	ecs_id_t *ids = type->array;
	int i = 0;
	while ((ecs_id_get_flags(world, ids[i]) & EcsIdOnInstantiateDontInherit)) {
		i ++;
	}
	const ecs_entity_t *result = ecs_bulk_new_w_id(world, ids[i], count);
	for (; i < type->count; i ++) {
		for (int e = 0; e < count; e ++) {
			if (ecs_id_get_flags(world, ids[i]) & EcsIdOnInstantiateDontInherit) {
				continue;
			}
			ecs_add_id(world, result[e], ids[i]);
		}
	}
    
	return result;
}

#endif // WITH
