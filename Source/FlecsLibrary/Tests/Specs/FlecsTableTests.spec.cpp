﻿
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS && defined(FLECS_TESTS)

#include "flecs.h"

#include "Bake/FlecsTestUtils.h"
#include "Bake/FlecsTestTypes.h"

BEGIN_DEFINE_SPEC(FFlecsTableTestsSpec,
                  "FlecsLibrary.Table",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

void Table_each(void) {
	flecs::world ecs;
    RegisterTestTypeComponents(ecs);

	ecs.entity().add<Position>();
	auto e2 = ecs.entity().add<Velocity>();

	ecs.query<Position>()
		.each([&](flecs::entity e, Position& p) {
			e2.add<Mass>();
		});

	test_assert(e2.has<Mass>());
}

void Table_each_without_entity(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    ecs.entity().add<Position>();
    auto e2 = ecs.entity().add<Velocity>();

    ecs.query<Position>()
        .each([&](Position& p) {
            e2.add<Mass>();
        });

    test_assert(e2.has<Mass>());
}

void Table_iter(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    ecs.entity().add<Position>();
    auto e2 = ecs.entity().add<Velocity>();

    ecs.query<Position>()
        .run([&](flecs::iter& it) {
            while (it.next()) {
                e2.add<Mass>();
            }
        });

    test_assert(e2.has<Mass>());
}

void Table_iter_without_components(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    ecs.entity().add<Position>();
    auto e2 = ecs.entity().add<Velocity>();

    ecs.query<Position>()
        .run([&](flecs::iter& it) {
            while (it.next()) {
                e2.add<Mass>();
            }
        });

    test_assert(e2.has<Mass>());
}

void Table_multi_get(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    auto e1 = ecs.entity().add<Position>().add<Velocity>();
    auto e2 = ecs.entity().add<Position>();

    test_bool(true, e1.get([&](const Position& p, const Velocity& v) {
        e2.add<Mass>();
    }));

    test_assert(e2.has<Mass>());
}

void Table_multi_set(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    auto e1 = ecs.entity().add<Position>().add<Velocity>();
    auto e2 = ecs.entity().add<Position>();

    e1.insert([&](Position& p, Velocity& v) {
        e2.add<Mass>();
    });

    test_assert(e2.has<Mass>());
}

void Table_count(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    flecs::entity e = ecs.entity().set<Position>({10, 20});
    ecs.entity().set<Position>({20, 30});
    ecs.entity().set<Position>({30, 40});

    flecs::table table = e.table();
    test_int(table.count(), 3);
}

void Table_has_id(void) {
    flecs::world ecs;
    
    flecs::entity t1 = ecs.entity();
    flecs::entity t2 = ecs.entity();
    flecs::entity t3 = ecs.entity();

    flecs::entity e = ecs.entity()
        .add(t1)
        .add(t2);
    ecs.entity()
        .add(t1)
        .add(t2);
    ecs.entity()
        .add(t1)
        .add(t2);

    flecs::table table = e.table();
    test_assert(table.has(t1));
    test_assert(table.has(t2));
    test_assert(!table.has(t3));
}

void Table_has_T(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    flecs::entity e = ecs.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});
    ecs.entity()
        .set<Position>({20, 30})
        .set<Velocity>({2, 3});
    ecs.entity()
        .set<Position>({30, 40})
        .set<Velocity>({3, 4});

    flecs::table table = e.table();
    
    test_assert(table.has<Position>());
    test_assert(table.has<Velocity>());
    test_assert(!table.has<Mass>());
}

void Table_has_pair_r_t(void) {
    flecs::world ecs;
    
    flecs::entity r = ecs.entity();
    flecs::entity t1 = ecs.entity();
    flecs::entity t2 = ecs.entity();
    flecs::entity t3 = ecs.entity();

    flecs::entity e = ecs.entity()
        .add(r, t1)
        .add(r, t2);
    ecs.entity()
        .add(r, t1)
        .add(r, t2);
    ecs.entity()
        .add(r, t1)
        .add(r, t2);

    flecs::table table = e.table();
    test_assert(table.has(r, t1));
    test_assert(table.has(r, t2));
    test_assert(!table.has(r, t3));
}

void Table_has_pair_R_t(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);
    
    flecs::entity t1 = ecs.entity();
    flecs::entity t2 = ecs.entity();
    flecs::entity t3 = ecs.entity();

    flecs::entity e = ecs.entity()
        .add<R>(t1)
        .add<R>(t2);
    ecs.entity()
        .add<R>(t1)
        .add<R>(t2);
    ecs.entity()
        .add<R>(t1)
        .add<R>(t2);

    flecs::table table = e.table();
    test_assert(table.has<R>(t1));
    test_assert(table.has<R>(t2));
    test_assert(!table.has<R>(t3));
}

void Table_has_pair_R_T(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    flecs::entity e = ecs.entity()
        .add<R,T1>()
        .add<R,T2>();
    ecs.entity()
        .add<R,T1>()
        .add<R,T2>();
    ecs.entity()
        .add<R,T1>()
        .add<R,T2>();

    flecs::table table = e.table();
    test_assert((table.has<R, T1>()));
    test_assert((table.has<R, T2>()));
    test_assert((!table.has<R, T3>()));
}

void Table_get_id(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    flecs::entity e = ecs.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});
    ecs.entity()
        .set<Position>({20, 30})
        .set<Velocity>({2, 3});
    ecs.entity()
        .set<Position>({30, 40})
        .set<Velocity>({3, 4});

    flecs::table table = e.table();
    void *ptr = table.get(ecs.id<Position>());
    Position *p = static_cast<Position*>(ptr);
    test_assert(p != NULL);
    test_int(p[0].x, 10);
    test_int(p[0].y, 20);
    test_int(p[1].x, 20);
    test_int(p[1].y, 30);
    test_int(p[2].x, 30);
    test_int(p[2].y, 40);

    ptr = table.get(ecs.id<Velocity>());
    Velocity *v = static_cast<Velocity*>(ptr);
    test_assert(v != NULL);
    test_int(v[0].x, 1);
    test_int(v[0].y, 2);
    test_int(v[1].x, 2);
    test_int(v[1].y, 3);
    test_int(v[2].x, 3);
    test_int(v[2].y, 4);
}

void Table_get_T(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    flecs::entity e = ecs.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});
    ecs.entity()
        .set<Position>({20, 30})
        .set<Velocity>({2, 3});
    ecs.entity()
        .set<Position>({30, 40})
        .set<Velocity>({3, 4});

    flecs::table table = e.table();
    Position *p = table.get<Position>();
    test_assert(p != NULL);
    test_int(p[0].x, 10);
    test_int(p[0].y, 20);
    test_int(p[1].x, 20);
    test_int(p[1].y, 30);
    test_int(p[2].x, 30);
    test_int(p[2].y, 40);

    Velocity *v = table.get<Velocity>();
    test_assert(v != NULL);
    test_int(v[0].x, 1);
    test_int(v[0].y, 2);
    test_int(v[1].x, 2);
    test_int(v[1].y, 3);
    test_int(v[2].x, 3);
    test_int(v[2].y, 4);
}

void Table_get_pair_r_t(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    flecs::entity e = ecs.entity()
        .set<Position, Tgt>({10, 20})
        .set<Velocity, Tgt>({1, 2});
    ecs.entity()
        .set<Position, Tgt>({20, 30})
        .set<Velocity, Tgt>({2, 3});
    ecs.entity()
        .set<Position, Tgt>({30, 40})
        .set<Velocity, Tgt>({3, 4});

    flecs::table table = e.table();
    void *ptr = table.get(ecs.id<Position>(), ecs.id<Tgt>());
    Position *p = static_cast<Position*>(ptr);
    test_assert(p != NULL);
    test_int(p[0].x, 10);
    test_int(p[0].y, 20);
    test_int(p[1].x, 20);
    test_int(p[1].y, 30);
    test_int(p[2].x, 30);
    test_int(p[2].y, 40);

    ptr = table.get(ecs.id<Velocity>(), ecs.id<Tgt>());
    Velocity *v = static_cast<Velocity*>(ptr);
    test_assert(v != NULL);
    test_int(v[0].x, 1);
    test_int(v[0].y, 2);
    test_int(v[1].x, 2);
    test_int(v[1].y, 3);
    test_int(v[2].x, 3);
    test_int(v[2].y, 4);
}

void Table_get_pair_R_t(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    flecs::entity e = ecs.entity()
        .set<Position, Tgt>({10, 20})
        .set<Velocity, Tgt>({1, 2});
    ecs.entity()
        .set<Position, Tgt>({20, 30})
        .set<Velocity, Tgt>({2, 3});
    ecs.entity()
        .set<Position, Tgt>({30, 40})
        .set<Velocity, Tgt>({3, 4});

    flecs::table table = e.table();
    void *ptr = table.get<Position>(ecs.id<Tgt>());
    Position *p = static_cast<Position*>(ptr);
    test_assert(p != NULL);
    test_int(p[0].x, 10);
    test_int(p[0].y, 20);
    test_int(p[1].x, 20);
    test_int(p[1].y, 30);
    test_int(p[2].x, 30);
    test_int(p[2].y, 40);

    ptr = table.get<Velocity>(ecs.id<Tgt>());
    Velocity *v = static_cast<Velocity*>(ptr);
    test_assert(v != NULL);
    test_int(v[0].x, 1);
    test_int(v[0].y, 2);
    test_int(v[1].x, 2);
    test_int(v[1].y, 3);
    test_int(v[2].x, 3);
    test_int(v[2].y, 4);
}

void Table_get_pair_R_T(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    flecs::entity e = ecs.entity()
        .set<Position, Tgt>({10, 20})
        .set<Velocity, Tgt>({1, 2});
    ecs.entity()
        .set<Position, Tgt>({20, 30})
        .set<Velocity, Tgt>({2, 3});
    ecs.entity()
        .set<Position, Tgt>({30, 40})
        .set<Velocity, Tgt>({3, 4});

    flecs::table table = e.table();
    void *ptr = table.get<Position, Tgt>();
    Position *p = static_cast<Position*>(ptr);
    test_assert(p != NULL);
    test_int(p[0].x, 10);
    test_int(p[0].y, 20);
    test_int(p[1].x, 20);
    test_int(p[1].y, 30);
    test_int(p[2].x, 30);
    test_int(p[2].y, 40);

    ptr = table.get<Velocity, Tgt>();
    Velocity *v = static_cast<Velocity*>(ptr);
    test_assert(v != NULL);
    test_int(v[0].x, 1);
    test_int(v[0].y, 2);
    test_int(v[1].x, 2);
    test_int(v[1].y, 3);
    test_int(v[2].x, 3);
    test_int(v[2].y, 4);
}

void Table_get_depth(void) {
    flecs::world world;

    flecs::entity e1 = world.entity();
    flecs::entity e2 = world.entity().child_of(e1);
    flecs::entity e3 = world.entity().child_of(e2);
    flecs::entity e4 = world.entity().child_of(e3);

    test_int(1, e2.table().depth(flecs::ChildOf));
    test_int(2, e3.table().depth(flecs::ChildOf));
    test_int(3, e4.table().depth(flecs::ChildOf));
}

void Table_get_depth_w_type(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);
    
    world.component<Rel>().add(flecs::Traversable);

    flecs::entity e1 = world.entity();
    flecs::entity e2 = world.entity().add<Rel>(e1);
    flecs::entity e3 = world.entity().add<Rel>(e2);
    flecs::entity e4 = world.entity().add<Rel>(e3);

    test_int(1, e2.table().depth<Rel>());
    test_int(2, e3.table().depth<Rel>());
    test_int(3, e4.table().depth<Rel>());
}

void Table_iter_type(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    auto e = ecs.entity()
        .add<Position>()
        .add<Velocity>();

    auto table = e.table();

    int32_t count = 0;
    for (const auto id : table.type()) {
        count ++;
        test_assert(id == ecs.id<Position>() || id == ecs.id<Velocity>());
    }
    test_int(count, 2);
}

void Table_get_T_enum(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    flecs::entity e = ecs.entity()
        .set<Number>(Number::One);
    ecs.entity()
        .set<Number>(Number::Two);
    ecs.entity()
        .set<Number>(Number::Three);

    flecs::table table = e.table();

    Number *n = table.get<Number>();
    test_assert(n != NULL);
    test_int(n[0], Number::One);
    test_int(n[1], Number::Two);
    test_int(n[2], Number::Three);
}

void Table_get_size(void) {
    flecs::world ecs;
    ecs.component<Position>();

    flecs::entity e = ecs.entity().set<Position>({10, 20});
    ecs.entity().set<Position>({20, 30});
    ecs.entity().set<Position>({30, 40});

    flecs::table table = e.table();
    test_int(table.count(), 3);
    test_int(table.size(), 4);
}

void Table_get_entities(void) {
    flecs::world ecs;
    ecs.component<Position>();

    flecs::entity e1 = ecs.entity().set<Position>({10, 20});
    flecs::entity e2 = ecs.entity().set<Position>({20, 30});
    flecs::entity e3 = ecs.entity().set<Position>({30, 40});

    flecs::table table = e1.table();
    const flecs::entity_t *entities = table.entities();
    test_assert(entities != NULL);
    test_uint(entities[0], e1);
    test_uint(entities[1], e2);
    test_uint(entities[2], e3);
}

void Table_get_records(void) {
    flecs::world ecs;
    ecs.component<Position>();

    flecs::entity parent = ecs.entity();
    flecs::entity e = ecs.entity()
        .child_of(parent)
        .set<Position>({10, 20});

    flecs::table table = e.table();
    const flecs::table_records_t records = table.records();
    test_int(records.count, 6);
    {
        const flecs::table_record_t *tr = &records.array[0];
        flecs::component_record_t *cr = flecs_table_record_get_component(tr);
        test_assert(cr != nullptr);
        test_uint(flecs_component_get_id(cr), ecs.component<Position>());
    }
    {
        const flecs::table_record_t *tr = &records.array[1];
        flecs::component_record_t *cr = flecs_table_record_get_component(tr);
        test_assert(cr != nullptr);
        test_uint(flecs_component_get_id(cr), ecs.pair(flecs::ChildOf, parent));
    }
}

void Table_unlock(void) {
    flecs::world ecs;
    ecs.component<Position>();

    flecs::entity e1 = ecs.entity()
        .set<Position>({10, 20});

    flecs::entity e2 = ecs.entity();
        
    flecs::table table = e1.table();
    table.lock();
    table.unlock();

    e2.set<Position>({20, 30});
    test_assert(e2.has<Position>());
}

void Table_has_flags(void) {
    flecs::world ecs;
    ecs.component<Position>();

    flecs::entity parent = ecs.entity();
    flecs::entity e1 = ecs.entity()
        .child_of(parent)
        .set<Position>({10, 20});
        
    flecs::table table = e1.table();

    test_assert(!table.has_flags(EcsTableIsComplex));
    test_assert(table.has_flags(EcsTableHasPairs));
    test_assert(table.has_flags(EcsTableHasChildOf));
    test_assert(!table.has_flags(EcsTableHasIsA));
}

void Table_clear_entities(void) {
    flecs::world ecs;
    ecs.component<Position>();

    flecs::entity e1 = ecs.entity()
        .set<Position>({10, 20});
    flecs::entity e2 = ecs.entity()
        .set<Position>({30, 40});

    test_assert(e1.is_alive());
    test_assert(e2.is_alive());
        
    flecs::table table = e1.table();
    test_int(table.count(), 2);
    test_int(table.size(), 2);

    table.clear_entities();

    test_assert(!e1.is_alive());
    test_assert(!e2.is_alive());

    test_int(table.count(), 0);
    test_int(table.size(), 2);
}

END_DEFINE_SPEC(FFlecsTableTestsSpec);

/*"id": "Table",
"testcases": [
"each",
"each_locked",
"each_without_entity",
"each_without_entity_locked",
"iter",
"iter_locked",
"iter_without_components",
"iter_without_components_locked",
"multi_get",
"multi_get_locked",
"multi_set",
"multi_set_locked",
"count",
"has_id",
"has_T",
"has_pair_r_t",
"has_pair_R_t",
"has_pair_R_T",
"get_id",
"get_T",
"get_T_enum",
"get_pair_r_t",
"get_pair_R_t",
"get_pair_R_T",
"range_get_id",
"range_get_T",
"range_get_pair_r_t",
"range_get_pair_R_t",
"range_get_pair_R_T",
"get_depth",
"get_depth_w_type",
"iter_type",
"get_size"
"get_entities",
"get_records",
"unlock",
"has_flags",
"clear_entities"

]*/

void FFlecsTableTestsSpec::Define()
{
	It("Table_each", [&]() { Table_each(); });
    It("Table_each_without_entity", [&]() { Table_each_without_entity(); });
    It("Table_iter", [&]() { Table_iter(); });
    It("Table_iter_without_components", [&]() { Table_iter_without_components(); });
    It("Table_multi_get", [&]() { Table_multi_get(); });
    It("Table_multi_set", [&]() { Table_multi_set(); });
    It("Table_count", [&]() { Table_count(); });
    It("Table_has_id", [&]() { Table_has_id(); });
    It("Table_has_T_type", [&]() { Table_has_T(); });
    It("Table_has_pair_r_entity_t_entity", [&]() { Table_has_pair_r_t(); });
    It("Table_has_pair_R_type_t_entity", [&]() { Table_has_pair_R_t(); });
    It("Table_has_pair_R_type_T_type", [&]() { Table_has_pair_R_T(); });
    It("Table_get_id", [&]() { Table_get_id(); });
    It("Table_get_T_type", [&]() { Table_get_T(); });
    It("Table_get_pair_r_entity_t_entity", [&]() { Table_get_pair_r_t(); });
    It("Table_get_pair_R_type_t_entity", [&]() { Table_get_pair_R_t(); });
    It("Table_get_pair_R_type_T_type", [&]() { Table_get_pair_R_T(); });
    It("Table_get_depth", [&]() { Table_get_depth(); });
    It("Table_get_depth_w_type", [&]() { Table_get_depth_w_type(); });
    It("Table_iter_type", [&]() { Table_iter_type(); });
    It("Table_get_T_type_enum", [&]() { Table_get_T_enum(); });
    It("Table_get_size", [&]() { Table_get_size(); });
    It("Table_get_entities", [&]() { Table_get_entities(); });
    It("Table_get_records", [&]() { Table_get_records(); });
    It("Table_unlock", [&]() { Table_unlock(); });
    It("Table_has_flags", [&]() { Table_has_flags(); });
    It("Table_clear_entities", [&]() { Table_clear_entities(); });
}

#endif // WITH_AUTOMATION_TESTS
