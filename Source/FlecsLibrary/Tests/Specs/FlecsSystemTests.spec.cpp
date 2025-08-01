﻿#if WITH_AUTOMATION_TESTS && defined(FLECS_TESTS)

#include "flecs.h"

#include "Misc/AutomationTest.h"
#include "Bake/FlecsTestUtils.h"
#include "Bake/FlecsTestTypes.h"

struct MyTag { };

struct TestFlecsEntity {
    flecs::entity e;
};

struct TestFlecsPipelineType {};
struct TestFlecsFirst {};
struct TestFlecsSecond {};

enum class PipelineStepEnum
{
    CustomStep,
    CustomStep2
};

BEGIN_DEFINE_SPEC(FFlecsSystemTestsSpec,
                  "FlecsLibrary.System",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);


void System_iter(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    auto entity = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    world.system<Position, Velocity>()
        .run([](flecs::iter& it) {
            while (it.next()) {
                auto p = it.field<Position>(0);
                auto v = it.field<Velocity>(1);
                for (auto i : it) {
                    p[i].x += v[i].x;
                    p[i].y += v[i].y;
                }
            }
        });

    world.progress();

    const Position *p = entity.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);

    const Velocity *v = entity.try_get<Velocity>();
    test_int(v->x, 1);
    test_int(v->y, 2);
}

void System_iter_const(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    auto entity = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    world.system<Position, const Velocity>()
        .run([](flecs::iter& it) {
            while (it.next()) {
                auto p = it.field<Position>(0);
                auto v = it.field<const Velocity>(1);
                for (auto i : it) {
                    p[i].x += v[i].x;
                    p[i].y += v[i].y;
                }
            }
        });

    world.progress();

    const Position *p = entity.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);

    const Velocity *v = entity.try_get<Velocity>();
    test_int(v->x, 1);
    test_int(v->y, 2);
}

void System_iter_shared(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    world.component<Position>().add(flecs::OnInstantiate, flecs::Inherit);
    world.component<Velocity>().add(flecs::OnInstantiate, flecs::Inherit);

    auto base = world.entity()
        .set<Velocity>({1, 2});

    auto e1 = world.entity()
        .set<Position>({10, 20})
        .add(flecs::IsA, base);

    auto e2 = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({3, 4});

    world.system<Position>().expr("Velocity(self|up IsA)")
        .run([](flecs::iter& it) {
            while (it.next()) {
                auto p = it.field<Position>(0);
                auto v = it.field<const Velocity>(1);

                if (!it.is_self(1)) {
                    for (auto i : it) {
                        p[i].x += v->x;
                        p[i].y += v->y;
                    }
                } else {
                    for (auto i : it) {
                        p[i].x += v[i].x;
                        p[i].y += v[i].y;
                    }
                }
            }
        });

    world.progress();

    const Position *p = e1.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);

    p = e2.try_get<Position>();
    test_int(p->x, 13);
    test_int(p->y, 24);
}

void System_iter_optional(void) {
    flecs::world world;
    flecs::component<Mass>(world, "Mass");
    RegisterTestTypeComponents(world);

    auto e1 = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2})
        .set<Mass>({1});

    auto e2 = world.entity()
        .set<Position>({30, 40})
        .set<Velocity>({3, 4})
        .set<Mass>({1});

    auto e3 = world.entity()
        .set<Position>({50, 60});

    auto e4 = world.entity()
        .set<Position>({70, 80});

    world.system<Position, Velocity*, Mass*>()
        .run([](flecs::iter& it) {
            while (it.next()) {
                auto p = it.field<Position>(0);
                auto v = it.field<Velocity>(1);
                auto m = it.field<Mass>(2);

                if (it.is_set(1) && it.is_set(2)) {
                    for (auto i : it) {
                        p[i].x += v[i].x * m[i].value;
                        p[i].y += v[i].y * m[i].value;
                    }
                } else {
                    for (auto i : it) {
                        p[i].x ++;
                        p[i].y ++;
                    }
                }
            }
    });

    world.progress();

    const Position *p = e1.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);

    p = e2.try_get<Position>();
    test_int(p->x, 33);
    test_int(p->y, 44);

    p = e3.try_get<Position>();
    test_int(p->x, 51);
    test_int(p->y, 61);

    p = e4.try_get<Position>();
    test_int(p->x, 71);
    test_int(p->y, 81);
}

void System_each(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    auto entity = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    world.system<Position, Velocity>()
        .each([](flecs::entity e, Position& p, Velocity& v) {
            p.x += v.x;
            p.y += v.y;
        });

    world.progress();

    const Position *p = entity.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);
}

void System_each_const(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    auto entity = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    world.system<Position, const Velocity>()
        .each([](flecs::entity e, Position& p, const Velocity& v) {
            p.x += v.x;
            p.y += v.y;
        });

    world.progress();

    const Position *p = entity.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);
}

void System_each_shared(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    auto base = world.entity()
        .set<Velocity>({1, 2});

    auto e1 = world.entity()
        .set<Position>({10, 20})
        .add(flecs::IsA, base);

    auto e2 = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({3, 4});

    world.system<Position, const Velocity>()
        .each([](flecs::entity e, Position& p, const Velocity& v) {
            p.x += v.x;
            p.y += v.y;
        });

    world.progress();

    const Position *p = e1.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);

    p = e2.try_get<Position>();
    test_int(p->x, 13);
    test_int(p->y, 24);
}

void System_each_optional(void) {
    flecs::world world;
    flecs::component<Mass>(world, "Mass");
    RegisterTestTypeComponents(world);

    auto e1 = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2})
        .set<Mass>({1});

    auto e2 = world.entity()
        .set<Position>({30, 40})
        .set<Velocity>({3, 4})
        .set<Mass>({1});

    auto e3 = world.entity()
        .set<Position>({50, 60});

    auto e4 = world.entity()
        .set<Position>({70, 80});

    world.system<Position, Velocity*, Mass*>()
        .each([](flecs::entity e, Position& p, Velocity* v, Mass *m) {
        if (v && m) {
            p.x += v->x * m->value;
            p.y += v->y * m->value;
        } else {
            p.x ++;
            p.y ++;
        }
    });

    world.progress();

    const Position *p = e1.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);

    p = e2.try_get<Position>();
    test_int(p->x, 33);
    test_int(p->y, 44);

    p = e3.try_get<Position>();
    test_int(p->x, 51);
    test_int(p->y, 61);

    p = e4.try_get<Position>();
    test_int(p->x, 71);
    test_int(p->y, 81);
}

void System_signature(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    auto entity = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    world.system<>().expr("Position, Velocity")
        .run([](flecs::iter& it) {
            while (it.next()) {
                flecs::field<Position> p(it, 0);
                flecs::field<Velocity> v(it, 1);

                for (auto i : it) {
                    p[i].x += v[i].x;
                    p[i].y += v[i].y;
                }
            }
        });

    world.progress();

    const Position *p = entity.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);

    const Velocity *v = entity.try_get<Velocity>();
    test_int(v->x, 1);
    test_int(v->y, 2);
}

void System_signature_const(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    auto entity = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    world.system<>().expr("Position, [in] Velocity")
        .run([](flecs::iter& it) {
            while (it.next()) {
                flecs::field<Position> p(it, 0);
                flecs::field<const Velocity> v(it, 1);

                for (auto i : it) {
                    p[i].x += v[i].x;
                    p[i].y += v[i].y;
                }
            }
        });

    world.progress();

    const Position *p = entity.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);

    const Velocity *v = entity.try_get<Velocity>();
    test_int(v->x, 1);
    test_int(v->y, 2);
}

void System_signature_shared(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    world.component<Position>().add(flecs::OnInstantiate, flecs::Inherit);
    world.component<Velocity>().add(flecs::OnInstantiate, flecs::Inherit);

    auto base = world.entity()
        .set<Velocity>({1, 2});

    auto e1 = world.entity()
        .set<Position>({10, 20})
        .add(flecs::IsA, base);

    auto e2 = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({3, 4});

    world.system<>().expr("Position, [in] Velocity(self|up IsA)")
        .run([](flecs::iter& it) {
            while (it.next()) {
                flecs::field<Position> p(it, 0);
                flecs::field<const Velocity> v(it, 1);

                if (!it.is_self(1)) {
                    for (auto i : it) {
                        p[i].x += v->x;
                        p[i].y += v->y;
                    }
                } else {
                    for (auto i : it) {
                        p[i].x += v[i].x;
                        p[i].y += v[i].y;
                    }
                }
            }
        });

    world.progress();

    const Position *p = e1.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);

    p = e2.try_get<Position>();
    test_int(p->x, 13);
    test_int(p->y, 24);
}

void System_signature_optional(void) {
    flecs::world world;
    flecs::component<Mass>(world, "Mass");

    RegisterTestTypeComponents(world);

    auto e1 = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2})
        .set<Mass>({1});

    auto e2 = world.entity()
        .set<Position>({30, 40})
        .set<Velocity>({3, 4})
        .set<Mass>({1});

    auto e3 = world.entity()
        .set<Position>({50, 60});

    auto e4 = world.entity()
        .set<Position>({70, 80});

    world.system<>().expr("Position, ?Velocity, ?Mass")
        .run([](flecs::iter& it) {
            while (it.next()) {
                flecs::field<Position> p(it, 0);
                flecs::field<Velocity> v(it, 1);
                flecs::field<Mass> m(it, 2);

                if (it.is_set(1) && it.is_set(2)) {
                    for (auto i : it) {
                        p[i].x += v[i].x * m[i].value;
                        p[i].y += v[i].y * m[i].value;
                    }
                } else {
                    for (auto i : it) {
                        p[i].x ++;
                        p[i].y ++;
                    }
                }
            }
        });

    world.progress();

    const Position *p = e1.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);

    p = e2.try_get<Position>();
    test_int(p->x, 33);
    test_int(p->y, 44);

    p = e3.try_get<Position>();
    test_int(p->x, 51);
    test_int(p->y, 61);

    p = e4.try_get<Position>();
    test_int(p->x, 71);
    test_int(p->y, 81);
}


void System_copy_name_on_create(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    char name[6];
    
    std::strcpy(name, "Hello");

    auto system_1 = world.system<Position>(name)
        .run([](flecs::iter& it) { while (it.next()) {} });

    std::strcpy(name, "World");
    
    auto system_2 = world.system<Position>(name)
        .run([](flecs::iter& it) { while (it.next()) {} });

    test_assert(system_1.id() != system_2.id());
}

void System_nested_system(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    auto system_1 = world.system<Position>("foo::bar")
        .run([](flecs::iter& it) { while (it.next()) {} });

    test_str(system_1.name().c_str(), "bar");

    auto e = world.lookup("foo");
    test_assert(e.id() != 0);
    test_str(e.name().c_str(), "foo");

    auto se = e.lookup("bar");
    test_assert(se.id() != 0);
    test_str(se.name().c_str(), "bar");
}

void System_empty_signature(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    int count = 0;

    world.system<>()
        .run([&](flecs::iter it) {
            while (it.next()) {
                count ++;
            }
        });

    world.progress();

    test_int(count, 1);
}

void System_iter_tag(void) {
    flecs::world world;
    world.component<MyTag>();

    int invoked = 0;

    world.system<MyTag>()
        .run([&](flecs::iter it) {
            while (it.next()) {
                invoked ++;
            }
        });

    world.entity()
        .add<MyTag>();

    world.progress();

    test_int(invoked, 1);
}


void System_each_tag(void) {
    flecs::world world;
    world.component<MyTag>();

    int invoked = 0;

    world.system<MyTag>()
        .each([&](flecs::entity e, MyTag) {
            invoked ++;
        });

    world.entity()
        .add<MyTag>();

    world.progress();

    test_int(invoked, 1);
}


void System_set_interval(void) {
    flecs::world world;

    auto sys = world.system<>()
        .kind(0)
        .interval(1.0f)
        .run([&](flecs::iter& it) { });

    float i = sys.interval();
    test_int(i, 1.0f);

    sys.interval(2.0f);

    i = sys.interval();
    test_int(i, 2.0f);
}

void System_order_by_type(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    world.entity().set<Position>({3, 0});
    world.entity().set<Position>({1, 0});
    world.entity().set<Position>({5, 0});
    world.entity().set<Position>({2, 0});
    world.entity().set<Position>({4, 0});

    float last_val = 0;
    int32_t count = 0;

    auto sys = world.system<const Position>()
        .order_by<Position>(
            [](flecs::entity_t e1, const Position *p1,
               flecs::entity_t e2, const Position *p2) {
                return (p1->x > p2->x) - (p1->x < p2->x);
            })
        .each([&](flecs::entity e, const Position& p) {
            test_assert(p.x > last_val);
            last_val = p.x;
            count ++;
        });

    sys.run();

    test_int(count, 5);
}

void System_order_by_id(void) {
    flecs::world world;

    auto pos = world.component<Position>();

    world.entity().set<Position>({3, 0});
    world.entity().set<Position>({1, 0});
    world.entity().set<Position>({5, 0});
    world.entity().set<Position>({2, 0});
    world.entity().set<Position>({4, 0});

    float last_val = 0;
    int32_t count = 0;

    auto sys = world.system<const Position>()
        .order_by(pos, [](flecs::entity_t e1, const void *p1,
                          flecs::entity_t e2, const void *p2)
            {
                return (static_cast<const Position*>(p1)->x >
                            static_cast<const Position*>(p2)->x) -
                       (static_cast<const Position*>(p1)->x <
                            static_cast<const Position*>(p2)->x);
            })
        .each([&](flecs::entity e, const Position& p) {
            test_assert(p.x > last_val);
            last_val = p.x;
            count ++;
        });

    sys.run();

    test_int(count, 5);
}

void System_order_by_type_after_create(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    world.entity().set<Position>({3, 0});
    world.entity().set<Position>({1, 0});
    world.entity().set<Position>({5, 0});
    world.entity().set<Position>({2, 0});
    world.entity().set<Position>({4, 0});

    float last_val = 0;
    int32_t count = 0;

    auto sys = world.system<const Position>()
        .order_by<Position>([](flecs::entity_t e1, const Position *p1,
            flecs::entity_t e2, const Position *p2) {
            return (p1->x > p2->x) - (p1->x < p2->x);
        })
        .each([&](flecs::entity e, const Position& p) {
            test_assert(p.x > last_val);
            last_val = p.x;
            count ++;
        });

    sys.run();

    test_int(count, 5);
}

void System_order_by_id_after_create(void) {
    flecs::world world;

    auto pos = world.component<Position>();

    world.entity().set<Position>({3, 0});
    world.entity().set<Position>({1, 0});
    world.entity().set<Position>({5, 0});
    world.entity().set<Position>({2, 0});
    world.entity().set<Position>({4, 0});

    float last_val = 0;
    int32_t count = 0;

    auto sys = world.system<const Position>()
        .order_by(pos, [](flecs::entity_t e1, const void *p1, flecs::entity_t e2, const void *p2) {
            return (static_cast<const Position*>(p1)->x > static_cast<const Position*>(p2)->x) -
                (static_cast<const Position*>(p1)->x < static_cast<const Position*>(p2)->x);
        })
        .each([&](flecs::entity e, const Position& p) {
            test_assert(p.x > last_val);
            last_val = p.x;
            count ++;
        });

    sys.run();

    test_int(count, 5);
}

void System_get_query(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    world.entity().set<Position>({0, 0});
    world.entity().set<Position>({1, 0});
    world.entity().set<Position>({2, 0});

    int32_t count = 0;

    auto sys = world.system<const Position>()
        .each([&](flecs::entity e, const Position& p) {
            // Not used
        });

    auto q = sys.query();

    q.run([&](flecs::iter &it) {
        while (it.next()) {
            auto pos = it.field<const Position>(0);
            for (auto i : it) {
                test_int(i, pos[i].x);
                count ++;
            }
        }
    });

    test_int(count, 3);
}

void System_add_from_each(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    auto e1 = world.entity().set<Position>({0, 0});
    auto e2 = world.entity().set<Position>({1, 0});
    auto e3 = world.entity().set<Position>({2, 0});

    world.system<const Position>()
        .each([](flecs::entity e, const Position& p) {
            e.add<Velocity>();
            // Add is deferred
            test_assert(!e.has<Velocity>());
        });

    world.progress();

    test_assert(e1.has<Velocity>());
    test_assert(e2.has<Velocity>());
    test_assert(e3.has<Velocity>());
}

void System_delete_from_each(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    auto e1 = world.entity().set<Position>({0, 0});
    auto e2 = world.entity().set<Position>({1, 0});
    auto e3 = world.entity().set<Position>({2, 0});

    world.system<const Position>()
        .each([](flecs::entity e, const Position& p) {
            e.destruct();
            // Delete is deferred
            test_assert(e.is_alive());
        });

    world.progress();

    test_assert(!e1.is_alive());
    test_assert(!e2.is_alive());
    test_assert(!e3.is_alive());
}

void System_add_from_each_world_handle(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);
    world.component<TestFlecsEntity>();

    auto e1 = world.entity().set<TestFlecsEntity>({world.entity()});
    auto e2 = world.entity().set<TestFlecsEntity>({world.entity()});
    auto e3 = world.entity().set<TestFlecsEntity>({world.entity()});

    world.system<const TestFlecsEntity>()
        .each([](flecs::entity e, const TestFlecsEntity& c) {
            c.e.mut(e).add<Position>();
        });

    world.progress();

    test_assert(e1.try_get<TestFlecsEntity>()->e.has<Position>());
    test_assert(e2.try_get<TestFlecsEntity>()->e.has<Position>());
    test_assert(e3.try_get<TestFlecsEntity>()->e.has<Position>());
}


void System_new_from_each(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);
    world.component<TestFlecsEntity>();

    auto e1 = world.entity().set<Position>({0, 0});
    auto e2 = world.entity().set<Position>({0, 0});
    auto e3 = world.entity().set<Position>({0, 0});

    world.system<const Position>()
        .each([](flecs::entity e, const Position& p) {
            e.set<TestFlecsEntity>({
                e.world().entity().add<Velocity>()
            });
        });

    world.progress();

    test_assert(e1.has<TestFlecsEntity>());
    test_assert(e2.has<TestFlecsEntity>());
    test_assert(e3.has<TestFlecsEntity>());

    test_assert(e1.try_get<TestFlecsEntity>()->e.has<Velocity>());
    test_assert(e2.try_get<TestFlecsEntity>()->e.has<Velocity>());
    test_assert(e3.try_get<TestFlecsEntity>()->e.has<Velocity>());
}


void System_add_from_iter(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    auto e1 = world.entity().set<Position>({0, 0});
    auto e2 = world.entity().set<Position>({1, 0});
    auto e3 = world.entity().set<Position>({2, 0});

    world.system<const Position>()
        .run([](flecs::iter& it) {
            while (it.next()) {
                for (auto i : it) {
                    it.entity(i).add<Velocity>();
                    test_assert(!it.entity(i).has<Velocity>());
                }
            }
        });

    world.progress();

    test_assert(e1.has<Velocity>());
    test_assert(e2.has<Velocity>());
    test_assert(e3.has<Velocity>());
}

void System_delete_from_iter(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    auto e1 = world.entity().set<Position>({0, 0});
    auto e2 = world.entity().set<Position>({1, 0});
    auto e3 = world.entity().set<Position>({2, 0});

    world.system<const Position>()
        .run([](flecs::iter& it) {
            while (it.next()) {
                for (auto i : it) {
                    it.entity(i).destruct();
                    // Delete is deferred
                    test_assert(it.entity(i).is_alive());
                }
            }
        });

    world.progress();

    test_assert(!e1.is_alive());
    test_assert(!e2.is_alive());
    test_assert(!e3.is_alive());
}

void System_add_from_iter_world_handle(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);
    world.component<TestFlecsEntity>();

    auto e1 = world.entity().set<TestFlecsEntity>({world.entity()});
    auto e2 = world.entity().set<TestFlecsEntity>({world.entity()});
    auto e3 = world.entity().set<TestFlecsEntity>({world.entity()});

    world.system<const TestFlecsEntity>()
        .run([](flecs::iter& it) {
            while (it.next()) {
                auto c = it.field<const TestFlecsEntity>(0);
                for (auto i : it) {
                    c[i].e.mut(it).add<Position>();
                }
            }
        });

    world.progress();

    test_assert(e1.try_get<TestFlecsEntity>()->e.has<Position>());
    test_assert(e2.try_get<TestFlecsEntity>()->e.has<Position>());
    test_assert(e3.try_get<TestFlecsEntity>()->e.has<Position>());
}

void System_new_from_iter(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);
    world.component<TestFlecsEntity>();

    auto e1 = world.entity().set<Position>({0, 0});
    auto e2 = world.entity().set<Position>({0, 0});
    auto e3 = world.entity().set<Position>({0, 0});

    world.system<const Position>()
        .run([](flecs::iter& it) {
            while (it.next()) {
                for (auto i : it) {
                    it.entity(i).set<TestFlecsEntity>({
                        it.world().entity().add<Velocity>()
                    });
                }
            }
        });

    world.progress();

    test_assert(e1.has<TestFlecsEntity>());
    test_assert(e2.has<TestFlecsEntity>());
    test_assert(e3.has<TestFlecsEntity>());

    test_assert(e1.try_get<TestFlecsEntity>()->e.has<Velocity>());
    test_assert(e2.try_get<TestFlecsEntity>()->e.has<Velocity>());
    test_assert(e3.try_get<TestFlecsEntity>()->e.has<Velocity>());
}

void System_each_w_mut_children_it(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    auto parent = world.entity().set<Position>({0, 0});
    auto e1 = world.entity().set<Position>({0, 0}).child_of(parent);
    auto e2 = world.entity().set<Position>({0, 0}).child_of(parent);
    auto e3 = world.entity().set<Position>({0, 0}).child_of(parent);

    int32_t count = 0;

    world.system<const Position>()
        .run([&](flecs::iter& it) {
            while (it.next()) {
                for (auto i : it) {
                    it.entity(i).children([&](flecs::entity child) {
                        child.add<Velocity>();
                        count ++;
                    });
                }
            }
        });

    world.progress();

    test_int(count, 3);

    test_assert(e1.has<Velocity>());
    test_assert(e2.has<Velocity>());
    test_assert(e3.has<Velocity>());
}

void System_readonly_children_iter(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);
    world.component<TestFlecsEntity>();

    auto parent = world.entity();
    world.entity().set<TestFlecsEntity>({ parent });
    world.entity().set<Position>({1, 0}).child_of(parent);
    world.entity().set<Position>({1, 0}).child_of(parent);
    world.entity().set<Position>({1, 0}).child_of(parent);

    int32_t count = 0;

    world.system<const TestFlecsEntity>()
        .run([&](flecs::iter& it) {
            while (it.next()) {
                auto c = it.field<const TestFlecsEntity>(0);
                for (auto i : it) {
                    c[i].e.children([&](flecs::entity child) {
                        // Dummy code to ensure we can access the entity
                        const Position *p = child.try_get<Position>();
                        test_int(p->x, 1);
                        test_int(p->y, 0);

                        count ++;
                    });
                }
            }
        });

    world.progress();

    test_int(count, 3);
}

void System_rate_filter(void) {
    flecs::world world;

    int32_t
    root_count = 0, root_mult = 1,
    l1_a_count = 0, l1_a_mult = 1,
    l1_b_count = 0, l1_b_mult = 2,
    l1_c_count = 0, l1_c_mult = 3,
    l2_a_count = 0, l2_a_mult = 2,
    l2_b_count = 0, l2_b_mult = 4,
    frame_count = 0;

    auto root = world.system<>("root")
        .run([&](flecs::iter& it) {
            while (it.next()) {
                root_count ++;
            }
        });

    auto l1_a = world.system<>("l1_a")
        .rate(root, 1)
        .run([&](flecs::iter& it) {
            while (it.next()) {
                l1_a_count ++;
            }
        });

    auto l1_b = world.system<>("l1_b")
        .rate(root, 2)
        .run([&](flecs::iter& it) {
            while (it.next()) {
                l1_b_count ++;
            }
        });

    world.system<>("l1_c")
        .rate(root, 3)
        .run([&](flecs::iter& it) {
            while (it.next()) {
                l1_c_count ++;
            }
        });

    world.system<>("l2_a")
        .rate(l1_a, 2)
        .run([&](flecs::iter& it) {
            while (it.next()) {
                l2_a_count ++;
            }
        });

    world.system<>("l2_b")
        .rate(l1_b, 2)
        .run([&](flecs::iter& it) {
            while (it.next()) {
                l2_b_count ++;
            }
        });

    for (int i = 0; i < 30; i ++) {
        world.progress(); frame_count ++;
        test_int(root_count, frame_count / root_mult);
        test_int(l1_a_count, frame_count / l1_a_mult);
        test_int(l1_b_count, frame_count / l1_b_mult);
        test_int(l1_c_count, frame_count / l1_c_mult);
        test_int(l2_a_count, frame_count / l2_a_mult);
        test_int(l2_b_count, frame_count / l2_b_mult);
    }
}

void System_self_rate_filter(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    int32_t count = 0;

    world.system<Position>("sys")
        .rate(2)
        .each([&](Position & p){
            count ++;
        });

    world.entity().set<Position>({1.0, 2.0});

    for(int i = 0; i < 10; i++) {
        world.progress();
    }

    test_int(count, 5);
}

void System_update_rate_filter(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    int32_t
    root_count = 0, root_mult = 1,
    l1_count = 0, l1_mult = 2,
    l2_count = 0, l2_mult = 6,
    frame_count = 0;

    auto root = world.system<>("root")
        .run([&](flecs::iter& it) {
            while (it.next()) {
                root_count ++;
            }
        });

    auto l1 = world.system<>("l1")
        .rate(root, 2)
        .run([&](flecs::iter& it) {
            while (it.next()) {
                l1_count ++;
            }
        });

    world.system<>("l2")
        .rate(l1, 3)
        .run([&](flecs::iter& it) {
            while (it.next()) {
                l2_count ++;
            }
        });

    for (int i = 0; i < 12; i ++) {
        world.progress(); frame_count ++;
        test_int(root_count, frame_count / root_mult);
        test_int(l1_count, frame_count / l1_mult);
        test_int(l2_count, frame_count / l2_mult);
    }

    l1.rate(4); // Run twice as slow
    l1_mult *= 2;
    l2_mult *= 2;

    frame_count = 0;
    l1_count = 0;
    l2_count = 0;
    root_count = 0;

    for (int i = 0; i < 32; i ++) {
        world.progress(); frame_count ++;
        test_int(root_count, frame_count / root_mult);
        test_int(l1_count, frame_count / l1_mult);
        test_int(l2_count, frame_count / l2_mult);
    }
}

void System_test_auto_defer_each(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    struct Value { int value; };
    world.component<Value>();

    auto e1 = world.entity().add<Tag>().set<Value>({10});
    auto e2 = world.entity().add<Tag>().set<Value>({20});
    auto e3 = world.entity().add<Tag>().set<Value>({30});

    auto s = world.system<Value>()
        .with<Tag>()
        .each([](flecs::entity e, Value& v) {
            v.value ++;
            e.remove<Tag>();
        });

    s.run();

    test_assert(!e1.has<Tag>());
    test_assert(!e2.has<Tag>());
    test_assert(!e3.has<Tag>());

    test_assert(e1.has<Value>());
    test_assert(e2.has<Value>());
    test_assert(e3.has<Value>());

    test_int(e1.try_get<Value>()->value, 11);
    test_int(e2.try_get<Value>()->value, 21);
    test_int(e3.try_get<Value>()->value, 31);
}

void System_test_auto_defer_iter(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    struct Value { int value; };
    world.component<Value>();

    auto e1 = world.entity().add<Tag>().set<Value>({10});
    auto e2 = world.entity().add<Tag>().set<Value>({20});
    auto e3 = world.entity().add<Tag>().set<Value>({30});

    auto s = world.system<Value>()
        .with<Tag>()
        .run([](flecs::iter& it) {
            while (it.next()) {
                auto v = it.field<Value>(0);
                for (auto i : it) {
                    v[i].value ++;
                    it.entity(i).remove<Tag>();
                }
            }
        });

    s.run();

    test_assert(!e1.has<Tag>());
    test_assert(!e2.has<Tag>());
    test_assert(!e3.has<Tag>());

    test_assert(e1.has<Value>());
    test_assert(e2.has<Value>());
    test_assert(e3.has<Value>());

    test_int(e1.try_get<Value>()->value, 11);
    test_int(e2.try_get<Value>()->value, 21);
    test_int(e3.try_get<Value>()->value, 31);
}

void System_custom_pipeline(void) {
    flecs::world world;

    auto PreFrame = world.entity().add(flecs::Phase);
    auto OnFrame = world.entity().add(flecs::Phase).depends_on(PreFrame);
    auto PostFrame = world.entity().add(flecs::Phase).depends_on(OnFrame);
    auto Tag = world.entity();

    flecs::entity pip = world.pipeline()
        .with(flecs::System)
        .with(flecs::Phase).cascade(flecs::DependsOn)
        .with(Tag)
        .build();

    int count = 0;

    world.system()
        .kind(PostFrame)
        .run([&](flecs::iter it) {
            while (it.next()) {
                test_int(count, 2);
                count ++;
            }
        })
        .add(Tag);

    world.system()
        .kind(OnFrame)
        .run([&](flecs::iter it) {
            while (it.next()) {
                test_int(count, 1);
                count ++;
            }
        })
        .add(Tag);

    world.system()
        .kind(PreFrame)
        .run([&](flecs::iter it) {
            while (it.next()) {
                test_int(count, 0);
                count ++;
            }
        })
        .add(Tag);

    test_int(count, 0);

    world.set_pipeline(pip);

    world.progress();

    test_int(count, 3);
}

void System_custom_pipeline_w_kind(void) {
    flecs::world world;

    auto Tag = world.entity();

    flecs::entity pip = world.pipeline()
        .with(flecs::System)
        .with(Tag)
        .build();

    int count = 0;

    world.system()
        .kind(Tag)
        .run([&](flecs::iter it) {
            while (it.next()) {
                test_int(count, 0);
                count ++;
            }
        });

    world.system()
        .kind(Tag)
        .run([&](flecs::iter it) {
            while (it.next()) {
                test_int(count, 1);
                count ++;
            }
        });

    world.system()
        .kind(Tag)
        .run([&](flecs::iter it) {
            while (it.next()) {
                test_int(count, 2);
                count ++;
            }
        });

    test_int(count, 0);

    world.set_pipeline(pip);

    world.progress();

    test_int(count, 3);
}

void System_instanced_query_w_singleton_each(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    ecs.component<Velocity>().add(flecs::Singleton);

    ecs.set<Velocity>({1, 2});

    auto e1 = ecs.entity().set<Position>({10, 20}); e1.set<Self>({e1});
    auto e2 = ecs.entity().set<Position>({20, 30}); e2.set<Self>({e2});
    auto e3 = ecs.entity().set<Position>({30, 40}); e3.set<Self>({e3});
    auto e4 = ecs.entity().set<Position>({40, 50}); e4.set<Self>({e4});
    auto e5 = ecs.entity().set<Position>({50, 60}); e5.set<Self>({e5});

    e4.add<Tag>();
    e5.add<Tag>();

    int32_t count = 0;

    auto s = ecs.system<Self, Position, const Velocity>()
        .each([&](flecs::entity e, Self& s, Position&p, const Velocity& v) {
            test_assert(e == s.value);
            p.x += v.x;
            p.y += v.y;
            count ++;
        });

    s.run();

    test_int(count, 5);

    test_assert(e1.get([](const Position& p) {
        test_int(p.x, 11);
        test_int(p.y, 22);
    }));

    test_assert(e2.get([](const Position& p) {
        test_int(p.x, 21);
        test_int(p.y, 32);
    }));

    test_assert(e3.get([](const Position& p) {
        test_int(p.x, 31);
        test_int(p.y, 42);
    }));

    test_assert(e4.get([](const Position& p) {
        test_int(p.x, 41);
        test_int(p.y, 52);
    }));

    test_assert(e5.get([](const Position& p) {
        test_int(p.x, 51);
        test_int(p.y, 62);
    }));
}

void System_instanced_query_w_base_each(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    auto base = ecs.entity().set<Velocity>({1, 2});

    auto e1 = ecs.entity().is_a(base).set<Position>({10, 20}); e1.set<Self>({e1});
    auto e2 = ecs.entity().is_a(base).set<Position>({20, 30}); e2.set<Self>({e2});
    auto e3 = ecs.entity().is_a(base).set<Position>({30, 40}); e3.set<Self>({e3});
    auto e4 = ecs.entity().is_a(base).set<Position>({40, 50}).add<Tag>(); e4.set<Self>({e4});
    auto e5 = ecs.entity().is_a(base).set<Position>({50, 60}).add<Tag>(); e5.set<Self>({e5});
    auto e6 = ecs.entity().set<Position>({60, 70}).set<Velocity>({2, 3}); e6.set<Self>({e6});
    auto e7 = ecs.entity().set<Position>({70, 80}).set<Velocity>({4, 5}); e7.set<Self>({e7});

    int32_t count = 0;
    auto s = ecs.system<Self, Position, const Velocity>()
        .each([&](flecs::entity e, Self& s, Position&p, const Velocity& v) {
            test_assert(e == s.value);
            p.x += v.x;
            p.y += v.y;
            count ++;
        });

    s.run();

    test_int(count, 7);

    test_assert(e1.get([](const Position& p) {
        test_int(p.x, 11);
        test_int(p.y, 22);
    }));

    test_assert(e2.get([](const Position& p) {
        test_int(p.x, 21);
        test_int(p.y, 32);
    }));

    test_assert(e3.get([](const Position& p) {
        test_int(p.x, 31);
        test_int(p.y, 42);
    }));

    test_assert(e4.get([](const Position& p) {
        test_int(p.x, 41);
        test_int(p.y, 52);
    }));

    test_assert(e5.get([](const Position& p) {
        test_int(p.x, 51);
        test_int(p.y, 62);
    }));

    test_assert(e6.get([](const Position& p) {
        test_int(p.x, 62);
        test_int(p.y, 73);
    }));

    test_assert(e7.get([](const Position& p) {
        test_int(p.x, 74);
        test_int(p.y, 85);
    }));
}

void System_instanced_query_w_singleton_iter(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    ecs.component<Velocity>().add(flecs::Singleton);

    ecs.set<Velocity>({1, 2});

    auto e1 = ecs.entity().set<Position>({10, 20}); e1.set<Self>({e1});
    auto e2 = ecs.entity().set<Position>({20, 30}); e2.set<Self>({e2});
    auto e3 = ecs.entity().set<Position>({30, 40}); e3.set<Self>({e3});
    auto e4 = ecs.entity().set<Position>({40, 50}); e4.set<Self>({e4});
    auto e5 = ecs.entity().set<Position>({50, 60}); e5.set<Self>({e5});

    e4.add<Tag>();
    e5.add<Tag>();

    int32_t count = 0;

    auto s = ecs.system<Self, Position, const Velocity>()
        .run([&](flecs::iter it) {
            while (it.next()) {
                auto s = it.field<Self>(0);
                auto p = it.field<Position>(1);
                auto v = it.field<const Velocity>(2);

                test_assert(it.count() > 1);

                for (auto i : it) {
                    p[i].x += v->x;
                    p[i].y += v->y;
                    test_assert(it.entity(i) == s[i].value);
                    count ++;
                }
            }
        });

    s.run();

    test_int(count, 5);

    test_assert(e1.get([](const Position& p) {
        test_int(p.x, 11);
        test_int(p.y, 22);
    }));

    test_assert(e2.get([](const Position& p) {
        test_int(p.x, 21);
        test_int(p.y, 32);
    }));

    test_assert(e3.get([](const Position& p) {
        test_int(p.x, 31);
        test_int(p.y, 42);
    }));

    test_assert(e4.get([](const Position& p) {
        test_int(p.x, 41);
        test_int(p.y, 52);
    }));

    test_assert(e5.get([](const Position& p) {
        test_int(p.x, 51);
        test_int(p.y, 62);
    }));
}

void System_instanced_query_w_base_iter(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    auto base = ecs.entity().set<Velocity>({1, 2});

    auto e1 = ecs.entity().is_a(base).set<Position>({10, 20}); e1.set<Self>({e1});
    auto e2 = ecs.entity().is_a(base).set<Position>({20, 30}); e2.set<Self>({e2});
    auto e3 = ecs.entity().is_a(base).set<Position>({30, 40}); e3.set<Self>({e3});
    auto e4 = ecs.entity().is_a(base).set<Position>({40, 50}).add<Tag>(); e4.set<Self>({e4});
    auto e5 = ecs.entity().is_a(base).set<Position>({50, 60}).add<Tag>(); e5.set<Self>({e5});
    auto e6 = ecs.entity().set<Position>({60, 70}).set<Velocity>({2, 3}); e6.set<Self>({e6});
    auto e7 = ecs.entity().set<Position>({70, 80}).set<Velocity>({4, 5}); e7.set<Self>({e7});

    int32_t count = 0;

    auto s = ecs.system<Self, Position, const Velocity>()
        .run([&](flecs::iter it) {
            while (it.next()) {
                auto s = it.field<Self>(0);
                auto p = it.field<Position>(1);
                auto v = it.field<const Velocity>(2);
                test_assert(it.count() > 1);

                for (auto i : it) {
                    if (it.is_self(2)) {
                        p[i].x += v[i].x;
                        p[i].y += v[i].y;
                    } else {
                        p[i].x += v->x;
                        p[i].y += v->y;
                    }

                    test_assert(it.entity(i) == s[i].value);
                    count ++;
                }
            }
        });

    s.run();

    test_int(count, 7);

    test_assert(e1.get([](const Position& p) {
        test_int(p.x, 11);
        test_int(p.y, 22);
    }));

    test_assert(e2.get([](const Position& p) {
        test_int(p.x, 21);
        test_int(p.y, 32);
    }));

    test_assert(e3.get([](const Position& p) {
        test_int(p.x, 31);
        test_int(p.y, 42);
    }));

    test_assert(e4.get([](const Position& p) {
        test_int(p.x, 41);
        test_int(p.y, 52);
    }));

    test_assert(e5.get([](const Position& p) {
        test_int(p.x, 51);
        test_int(p.y, 62);
    }));

    test_assert(e6.get([](const Position& p) {
        test_int(p.x, 62);
        test_int(p.y, 73);
    }));

    test_assert(e7.get([](const Position& p) {
        test_int(p.x, 74);
        test_int(p.y, 85);
    }));
}

void System_create_w_no_template_args(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    auto entity = world.entity()
        .set<Position>({10, 20});

    int32_t count = 0;

    auto s = world.system()
        .with<Position>()
        .each([&](flecs::entity e) {
            test_assert(e == entity);
            count ++;
        });

    s.run();

    test_int(count, 1);
}

void System_system_w_type_kind_type_pipeline(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);
    world.component<TestFlecsFirst>();
    world.component<TestFlecsSecond>();
    world.component<TestFlecsPipelineType>();

    world.component<TestFlecsSecond>()
        .add(flecs::Phase)
        .depends_on(
            world.component<TestFlecsFirst>()
                .add(flecs::Phase)
        );

    world.pipeline<TestFlecsPipelineType>()
        .with(flecs::System)
        .with(flecs::Phase).cascade(flecs::DependsOn)
        .build();

    world.set_pipeline<TestFlecsPipelineType>();

    auto entity = world.entity().add<Tag>();

    int32_t s1_count = 0;
    int32_t s2_count = 0;

    world.system<Tag>()
        .kind<TestFlecsSecond>()
        .each([&](flecs::entity e, Tag) {
            test_assert(e == entity);
            test_int(s1_count, 0);
            test_int(s2_count, 1);
            s1_count ++;
        });

    world.system<Tag>()
        .kind<TestFlecsFirst>()
        .each([&](flecs::entity e, Tag) {
            test_assert(e == entity);
            test_int(s1_count, 0);
            s2_count ++;
        });

    world.progress();

    test_int(s1_count, 1);
    test_int(s2_count, 1);
}

void System_default_ctor(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    flecs::system sys_var;

    int count = 0;
    auto sys = world.system<Position>()
        .each([&](flecs::entity e, Position& p) {
            test_int(p.x, 10);
            test_int(p.y, 20);
            count ++;
        });

    world.entity().set<Position>({10, 20});

    sys_var = sys;

    sys_var.run();

    test_int(count, 1);
}

void System_entity_ctor(void) {
    flecs::world world;

    uint32_t invoked = 0;

    flecs::entity sys = world.system<>()
        .run([&](flecs::iter& it) {
            while (it.next()) {
                invoked ++;
            }
        });

    auto sys_from_id = world.system(sys);

    sys_from_id.run();
    test_int(invoked, 1);
}

void System_ensure_instanced_w_each(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    flecs::entity e1 = world.entity().set<Position>({10, 20});

    int32_t count = 0;
    auto sys = world.system<Position>()
        .each([&](flecs::iter& it, size_t i, Position&) {
            test_assert(it.entity(i) == e1);
            count ++;
        });

    auto q = sys.query();

    test_int(count, 0);
    sys.run();
    test_int(count, 1);
}

void System_multithread_system_w_query_each(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    world.set_threads(2);

    flecs::entity e = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    auto q = world.query<Velocity>();

    world.system<Position>()
        .multi_threaded()
        .each([&](flecs::entity e, Position& p) {
            q.iter(e).each([&](Velocity& v) {
                p.x += v.x;
                p.y += v.y;
            });
        });

    world.progress();

    const Position *p = e.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);
}

void System_multithread_system_w_query_each_w_iter(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    world.set_threads(2);

    flecs::entity e = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    auto q = world.query<Velocity>();

    world.system<Position>()
        .multi_threaded()
        .each([&](flecs::iter& it, size_t, Position& p) {
            q.iter(it).each([&](Velocity& v) {
                p.x += v.x;
                p.y += v.y;
            });
        });

    world.progress();

    const Position *p = e.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);
}

void System_multithread_system_w_query_each_w_world(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    world.set_threads(2);

    flecs::entity e = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    auto q = world.query<Velocity>();

    world.system<Position>()
        .multi_threaded()
        .each([&](flecs::iter& it, size_t, Position& p) {
            q.iter(it.world()).each([&](Velocity& v) {
                p.x += v.x;
                p.y += v.y;
            });
        });

    world.progress();

    const Position *p = e.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);
}

void System_multithread_system_w_query_iter(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    world.set_threads(2);

    flecs::entity e = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    auto q = world.query<Velocity>();

    world.system<Position>()
        .multi_threaded()
        .each([&](flecs::entity e, Position& p) {
            q.iter(e).run([&](flecs::iter& it) {
                while (it.next()) {
                    auto v = it.field<Velocity>(0);

                    for (auto i : it) {
                        p.x += v[i].x;
                        p.y += v[i].y;
                    }
                }
            });
        });

    world.progress();

    const Position *p = e.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);
}

void System_multithread_system_w_query_iter_w_iter(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    world.set_threads(2);

    flecs::entity e = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    auto q = world.query<Velocity>();

    world.system<Position>()
        .multi_threaded()
        .each([&](flecs::iter& it, size_t, Position& p) {
            q.iter(it).run([&](flecs::iter& it) {
                while (it.next()) {
                    auto v = it.field<Velocity>(0);
                    for (auto i : it) {
                        p.x += v[i].x;
                        p.y += v[i].y;
                    }
                }
            });
        });

    world.progress();

    const Position *p = e.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);
}

void System_multithread_system_w_query_iter_w_world(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    world.set_threads(2);

    flecs::entity e = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    auto q = world.query<Velocity>();

    world.system<Position>()
        .multi_threaded()
        .each([&](flecs::iter& it, size_t, Position& p) {
            q.iter(it.world()).run([&](flecs::iter& it) {
                while (it.next()) {
                    auto v = it.field<Velocity>(0);
                    for (auto i : it) {
                        p.x += v[i].x;
                        p.y += v[i].y;
                    }
                }
            });
        });

    world.progress();

    const Position *p = e.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);
}

void System_multithread_system_w_get_var(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);
    
    world.set_threads(4);

    flecs::entity bob = world.entity("bob").add<Position>();
    flecs::entity alice = world.entity("alice").add<Position>();

    bob.add<Rel>(alice);

    int count = 0;

    world.system<const Position>()
        .with<Rel>("$other")
        .term_at(0).src("$other")
        .multi_threaded()
        .each([&](flecs::iter &it, size_t row, const Position &pos) {
            flecs::entity e = it.entity(row);
            flecs::entity other = it.get_var("other");

            test_assert(e == bob);
            test_assert(other == alice);
            
            count ++;
        });

    world.progress();

    test_int(count, 1);
}

void System_run_callback(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    auto entity = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    world.system<Position, const Velocity>()
        .run(
            [](flecs::iter& it) {
                while (it.next()) {
                    it.each();
                }
            },
            [](Position& p, const Velocity& v) {
                p.x += v.x;
                p.y += v.y;
            });

    world.progress();

    const Position *p = entity.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);

    const Velocity *v = entity.try_get<Velocity>();
    test_int(v->x, 1);
    test_int(v->y, 2);
}

void System_startup_system(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    int32_t count_a = 0, count_b = 0;

    ecs.system()
        .kind(flecs::OnStart)
        .run([&](flecs::iter& it) {
            while (it.next()) {
                test_assert(it.delta_time() == 0);
                count_a ++;
            }
        });

    ecs.system()
        .kind(flecs::OnUpdate)
        .run([&](flecs::iter& it) {
            while (it.next()) {
                test_assert(it.delta_time() != 0);
                count_b ++;
            }
        });

    ecs.progress();
    test_int(count_a, 1);
    test_int(count_b, 1);

    ecs.progress();
    test_int(count_a, 1);
    test_int(count_b, 2);
}

void System_interval_tick_source(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    flecs::timer t = ecs.timer().interval(2.1);

    flecs::Timer& timer = t.obtain<flecs::Timer>();
    timer.time = 0;

    int32_t sys_a_invoked = 0, sys_b_invoked = 0;

    ecs.system()
        .tick_source(t)
        .run([&](flecs::iter& it) {
            while (it.next()) {
                sys_a_invoked ++;
            }
        });

    ecs.system()
        .tick_source(t)
        .run([&](flecs::iter& it) {
            while (it.next()) {
                sys_b_invoked ++;
            }
        });

    ecs.progress(1.0);
    test_int(0, sys_a_invoked);
    test_int(0, sys_b_invoked);

    ecs.progress(1.0);
    test_int(0, sys_a_invoked);
    test_int(0, sys_b_invoked);

    ecs.progress(1.0);
    test_int(1, sys_a_invoked);
    test_int(1, sys_b_invoked);
}

void System_rate_tick_source(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    flecs::timer t = ecs.timer().rate(3);

    int32_t sys_a_invoked = 0, sys_b_invoked = 0;

    ecs.system()
        .tick_source(t)
        .run([&](flecs::iter& it) {
            while (it.next()) {
                sys_a_invoked ++;
            }
        });

    ecs.system()
        .tick_source(t)
        .run([&](flecs::iter& it) {
            while (it.next()) {
                sys_b_invoked ++;
            }
        });

    ecs.progress(1.0);
    test_int(0, sys_a_invoked);
    test_int(0, sys_b_invoked);

    ecs.progress(1.0);
    test_int(0, sys_a_invoked);
    test_int(0, sys_b_invoked);

    ecs.progress(1.0);
    test_int(1, sys_a_invoked);
    test_int(1, sys_b_invoked);
}

void System_nested_rate_tick_source(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    flecs::timer t_3 = ecs.timer().rate(3);
    flecs::timer t_6 = ecs.timer().rate(2, t_3);

    int32_t sys_a_invoked = 0, sys_b_invoked = 0;

    ecs.system()
        .tick_source(t_3)
        .run([&](flecs::iter& it) {
            while (it.next()) {
                sys_a_invoked ++;
            }
        });

    ecs.system()
        .tick_source(t_6)
        .run([&](flecs::iter& it) {
            while (it.next()) {
                sys_b_invoked ++;
            }
        });

    ecs.progress(1.0);
    test_int(0, sys_a_invoked);
    test_int(0, sys_b_invoked);

    ecs.progress(1.0);
    test_int(0, sys_a_invoked);
    test_int(0, sys_b_invoked);

    ecs.progress(1.0);
    test_int(1, sys_a_invoked);
    test_int(0, sys_b_invoked);

    ecs.progress(1.0);
    test_int(1, sys_a_invoked);
    test_int(0, sys_b_invoked);

    ecs.progress(1.0);
    test_int(1, sys_a_invoked);
    test_int(0, sys_b_invoked);

    ecs.progress(1.0);
    test_int(2, sys_a_invoked);
    test_int(1, sys_b_invoked);
}

void System_table_get(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    flecs::entity e1 = ecs.entity().set<Position>({10, 20});
    flecs::entity e2 = ecs.entity().set<Position>({20, 30});

    auto s = ecs.system()
        .with<Position>()
        .each([&](flecs::iter& iter, size_t index) {
            flecs::entity e = iter.entity(index);
            const Position *p = &iter.table().try_get<Position>()[index];
            test_assert(p != nullptr);
            test_assert(e == e1 || e == e2);
            if (e == e1) {
                test_int(p->x, 10);
                test_int(p->y, 20);
            } else if (e == e2) {
                test_int(p->x, 20);
                test_int(p->y, 30);
            }
        });

    s.run();
}

void System_range_get(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    flecs::entity e1 = ecs.entity().set<Position>({10, 20});
    flecs::entity e2 = ecs.entity().set<Position>({20, 30});

    auto s = ecs.system()
        .with<Position>()
        .each([&](flecs::iter& iter, size_t index) {
            flecs::entity e = iter.entity(index);
            const Position *p = &iter.range().try_get<Position>()[index];
            test_assert(p != nullptr);
            test_assert(e == e1 || e == e2);
            if (e == e1) {
                test_int(p->x, 10);
                test_int(p->y, 20);
            } else if (e == e2) {
                test_int(p->x, 20);
                test_int(p->y, 30);
            }
        });

    s.run();
}

void System_randomize_timers(void) {
    flecs::world ecs;

    flecs::entity s1 = ecs.system()
        .interval(1.0)
        .run([](flecs::iter& it) { while (it.next()) {} });

    {
        const flecs::Timer *t = s1.try_get<flecs::Timer>();
        test_assert(t != nullptr);
        test_assert(t->time == 0);
    }

    ecs.randomize_timers();

    flecs::entity s2 = ecs.system()
        .interval(1.0)
        .run([](flecs::iter& it) { while (it.next()) {} });

    {
        const flecs::Timer *t = s1.try_get<flecs::Timer>();
        test_assert(t != nullptr);
        test_assert(t->time != 0);
    }

    {
        const flecs::Timer *t = s2.try_get<flecs::Timer>();
        test_assert(t != nullptr);
        test_assert(t->time != 0);
    }
}

void System_optional_pair_term(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    ecs.entity()
        .add<Tag0>()
        .emplace<Position, Tag>(1.0f, 2.0f);
    ecs.entity()
        .add<Tag0>();

    int32_t with_pair = 0, without_pair = 0;

    ecs.system<flecs::pair<Position, Tag>*>()
        .with<Tag0>()
        .each([&](flecs::entity e, Position* p)
        {
            if (p)
            {
                with_pair++;
                test_flt(1.0f, p->x);
                test_flt(2.0f, p->y);
            }
            else
            {
                without_pair++;
            }
        });

    ecs.progress(1.0);

    test_int(1, with_pair);
    test_int(1, without_pair);
}

void System_singleton_tick_source(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    ecs.timer<Tag0>().timeout(1.5);

    int32_t sys_invoked = 0;

    ecs.system()
        .tick_source<Tag0>()
        .run([&](flecs::iter& it) {
            sys_invoked ++;
        });

    ecs.progress(1.0);
    test_int(0, sys_invoked);

    ecs.progress(1.0);
    test_int(1, sys_invoked);

    ecs.progress(2.0);
    test_int(1, sys_invoked);
}

void System_pipeline_step_with_kind_enum(void) {
    flecs::world ecs;
    ecs.component<PipelineStepEnum>();

    ecs.entity(PipelineStepEnum::CustomStep).add(flecs::Phase).depends_on(flecs::OnStart);

    bool ran_test = false;

    ecs.system().kind(PipelineStepEnum::CustomStep)
        .run([&ran_test](flecs::iter& it) {
            while (it.next()) {
                ran_test = true;
            }
        });

    ecs.progress();
    test_assert(ran_test);
}

void System_pipeline_step_depends_on_pipeline_step_with_enum(void) {
    flecs::world ecs;
    ecs.component<PipelineStepEnum>();

    ecs.entity(PipelineStepEnum::CustomStep).add(flecs::Phase).depends_on(flecs::OnStart);
    ecs.entity(PipelineStepEnum::CustomStep2).add(flecs::Phase).depends_on(PipelineStepEnum::CustomStep);

    bool ran_test = false;

    ecs.system().kind(PipelineStepEnum::CustomStep2)
        .run([&ran_test](flecs::iter& it) {
            while (it.next()) {
                ran_test = true;
            }
        });

    ecs.progress();
    test_assert(ran_test);
}

void System_register_twice_w_each(void) {
    flecs::world ecs;

    int count1 = 0, count2 = 0;

    flecs::system sys1 = ecs.system("Test")
        .each([&](flecs::iter&, size_t) {
            count1 ++;
        });

    sys1.run();
    test_int(count1, 1);

    flecs::system sys2 = ecs.system("Test")
        .each([&](flecs::iter&, size_t) {
            count2 ++;
        });

    sys2.run();
    test_int(count2, 1);
}

void System_register_twice_w_run(void) {
    flecs::world ecs;

    int count1 = 0, count2 = 0;

    flecs::system sys1 = ecs.system("Test")
        .run([&](flecs::iter&) {
            count1 ++;
        });

    sys1.run();
    test_int(count1, 1);

    flecs::system sys2 = ecs.system("Test")
        .run([&](flecs::iter&) {
            count2 ++;
        });

    sys2.run();
    test_int(count2, 1);
}

void System_register_twice_w_run_each(void) {
    flecs::world ecs;

    int count1 = 0, count2 = 0;

    flecs::system sys1 = ecs.system("Test")
        .run([&](flecs::iter&) {
            count1 ++;
        });

    sys1.run();
    test_int(count1, 1);

    flecs::system sys2 = ecs.system("Test")
        .each([&](flecs::iter&, size_t) {
            count2 ++;
        });

    sys2.run();
    test_int(count2, 1);
}

void System_register_twice_w_each_run(void) {
    flecs::world ecs;

    int count1 = 0, count2 = 0;

    flecs::system sys1 = ecs.system("Test")
        .each([&](flecs::iter&, size_t) {
            count1 ++;
        });

    sys1.run();
    test_int(count1, 1);

    flecs::system sys2 = ecs.system("Test")
        .run([&](flecs::iter&) {
            count2 ++;
        });

    sys2.run();
    test_int(count2, 1);
}

void System_run_w_0_src_query(void) {
    flecs::world world;
    RegisterTestTypeComponents(world);

    int count = 0;

    world.system()
        .write<Position>()
        .run([&](flecs::iter&){
            count ++;
        });

    world.progress();
    test_int(count, 1);
}

void System_priority_test(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    int32_t count = 0;

    // have system initialize out of order to make sure Ids are not used in ordering
    
    ecs.system()
       .priority(1000)
       .each([&](flecs::iter& Iter, size_t Index) {
           test_assert(count == 3);
           count ++;
       });

    ecs.system()
        .each([&](flecs::iter& Iter, size_t Index) {
            test_assert(count == 2);
            count ++;
        });
    
    ecs.system()
        .priority(99)
        .each([&](flecs::iter& Iter, size_t Index) {
            test_assert(count == 1);
            count ++;
        });

    ecs.system()
        .priority(50)
        .each([&](flecs::iter& Iter, size_t Index) {
            count ++;
        });

    ecs.progress();
    test_int(count, 4);
}


END_DEFINE_SPEC(FFlecsSystemTestsSpec);

/*"id": "System",
            "testcases": [
                "iter",
                "iter_const",
                "iter_shared",
                "iter_optional",
                "each",
                "each_const",
                "each_shared",
                "each_optional",
                "signature",
                "signature_const",
                "signature_shared",
                "signature_optional",
                "copy_name_on_create",
                "nested_system",
                "empty_signature",
                "iter_tag",
                "each_tag",
                "set_interval",
                "order_by_type",
                "order_by_id",
                "order_by_type_after_create",
                "order_by_id_after_create",
                "get_query",
                "add_from_each",
                "delete_from_each",
                "add_from_each_world_handle",
                "new_from_each",
                "add_from_iter",
                "delete_from_iter",
                "add_from_iter_world_handle",
                "new_from_iter",
                "each_w_mut_children_it",
                "readonly_children_iter",
                "rate_filter",
                "self_rate_filter",
                "update_rate_filter",
                "default_ctor",
                "test_auto_defer_each",
                "test_auto_defer_iter",
                "custom_pipeline",
                "custom_pipeline_w_kind",
                "instanced_query_w_singleton_each",
                "instanced_query_w_base_each",
                "instanced_query_w_singleton_iter",
                "instanced_query_w_base_iter",
                "create_w_no_template_args",
                "system_w_type_kind_type_pipeline",
                "default_ctor",
                "entity_ctor",
                "ensure_instanced_w_each",
                "multithread_system_w_query_each",
                "multithread_system_w_query_each_w_iter",
                "multithread_system_w_query_each_w_world",
                "multithread_system_w_query_iter",
                "multithread_system_w_query_iter_w_iter",
                "multithread_system_w_query_iter_w_world",
                "multithread_system_w_get_var",
                "run_callback",
                "startup_system",
                "interval_tick_source",
                "rate_tick_source",
                "nested_rate_tick_source",
                "table_get",
                "range_get",
                "randomize_timers",
                "optional_pair_term",
                "singleton_tick_source",
                "pipeline_step_with_kind_enum",
                "pipeline_step_depends_on_pipeline_step_with_enum",
                "register_twice_w_each",
                "register_twice_w_run",
                "register_twice_w_run_each",
                "register_twice_w_each_run",
                "run_w_0_src_query",
                "priority_test"
            ]*/

void FFlecsSystemTestsSpec::Define()
{
	It("iter", [&] { System_iter(); });
    It("iter_const", [&] { System_iter_const(); });
    It("iter_shared", [&] { System_iter_shared(); });
    It("iter_optional", [&] { System_iter_optional(); });
    It("each", [&] { System_each(); });
    It("each_const", [&] { System_each_const(); });
    It("each_shared", [&] { System_each_shared(); });
    It("each_optional", [&] { System_each_optional(); });
    It("signature", [&] { System_signature(); });
    It("signature_const", [&] { System_signature_const(); });
    It("signature_shared", [&] { System_signature_shared(); });
    It("signature_optional", [&] { System_signature_optional(); });
    It("copy_name_on_create", [&] { System_copy_name_on_create(); });
    It("nested_system", [&] { System_nested_system(); });
    It("empty_signature", [&] { System_empty_signature(); });
    It("iter_tag", [&] { System_iter_tag(); });
    It("each_tag", [&] { System_each_tag(); });
    It("set_interval", [&] { System_set_interval(); });
    It("order_by_type", [&] { System_order_by_type(); });
    It("order_by_id", [&] { System_order_by_id(); });
    It("order_by_type_after_create", [&] { System_order_by_type_after_create(); });
    It("order_by_id_after_create", [&] { System_order_by_id_after_create(); });
    It("get_query", [&] { System_get_query(); });
    It("add_from_each", [&] { System_add_from_each(); });
    It("delete_from_each", [&] { System_delete_from_each(); });
    It("add_from_each_world_handle", [&] { System_add_from_each_world_handle(); });
    It("new_from_each", [&] { System_new_from_each(); });
    It("add_from_iter", [&] { System_add_from_iter(); });
    It("delete_from_iter", [&] { System_delete_from_iter(); });
    It("add_from_iter_world_handle", [&] { System_add_from_iter_world_handle(); });
    It("new_from_iter", [&] { System_new_from_iter(); });
    It("each_w_mut_children_it", [&] { System_each_w_mut_children_it(); });
    It("readonly_children_iter", [&] { System_readonly_children_iter(); });
    It("rate_filter", [&] { System_rate_filter(); });
    It("self_rate_filter", [&] { System_self_rate_filter(); });
    It("update_rate_filter", [&] { System_update_rate_filter(); });
    It("test_auto_defer_each", [&] { System_test_auto_defer_each(); });
    It("test_auto_defer_iter", [&] { System_test_auto_defer_iter(); });
    It("custom_pipeline", [&] { System_custom_pipeline(); });
    It("custom_pipeline_w_kind", [&] { System_custom_pipeline_w_kind(); });
    It("instanced_query_w_singleton_each", [&] { System_instanced_query_w_singleton_each(); });
    It("instanced_query_w_base_each", [&] { System_instanced_query_w_base_each(); });
    It("instanced_query_w_singleton_iter", [&] { System_instanced_query_w_singleton_iter(); });
    It("instanced_query_w_base_iter", [&] { System_instanced_query_w_base_iter(); });
    It("create_w_no_template_args", [&] { System_create_w_no_template_args(); });
    It("system_w_type_kind_type_pipeline", [&] { System_system_w_type_kind_type_pipeline(); });
    It("default_ctor", [&] { System_default_ctor(); });
    It("entity_ctor", [&] { System_entity_ctor(); });
    It("ensure_instanced_w_each", [&] { System_ensure_instanced_w_each(); });
    It("multithread_system_w_query_each", [&] { System_multithread_system_w_query_each(); });
    It("multithread_system_w_query_each_w_iter", [&] { System_multithread_system_w_query_each_w_iter(); });
    It("multithread_system_w_query_each_w_world", [&] { System_multithread_system_w_query_each_w_world(); });
    It("multithread_system_w_query_iter", [&] { System_multithread_system_w_query_iter(); });
    It("multithread_system_w_query_iter_w_iter", [&] { System_multithread_system_w_query_iter_w_iter(); });
    It("multithread_system_w_query_iter_w_world", [&] { System_multithread_system_w_query_iter_w_world(); });
    It("multithread_system_w_get_var", [&] { System_multithread_system_w_get_var(); });
    It("run_callback", [&] { System_run_callback(); });
    It("startup_system", [&] { System_startup_system(); });
    It("interval_tick_source", [&] { System_interval_tick_source(); });
    It("rate_tick_source", [&] { System_rate_tick_source(); });
    It("nested_rate_tick_source", [&] { System_nested_rate_tick_source(); });
    It("table_get", [&] { System_table_get(); });
    It("range_get", [&] { System_range_get(); });
    It("randomize_timers", [&] { System_randomize_timers(); });
    It("optional_pair_term", [&] { System_optional_pair_term(); });
    It("singleton_tick_source", [&] { System_singleton_tick_source(); });
    It("pipeline_step_with_kind_enum", [&] { System_pipeline_step_with_kind_enum(); });
    It("pipeline_step_depends_on_pipeline_step_with_enum", [&] { System_pipeline_step_depends_on_pipeline_step_with_enum(); });
    It("register_twice_w_each", [&] { System_register_twice_w_each(); });
    It("register_twice_w_run", [&] { System_register_twice_w_run(); });
    It("register_twice_w_run_each", [&] { System_register_twice_w_run_each(); });
    It("register_twice_w_each_run", [&] { System_register_twice_w_each_run(); });
    It("run_w_0_src_query", [&] { System_run_w_0_src_query(); });
    It("priority_test", [&] { System_priority_test(); });
}

#endif // WITH_AUTOMATION_TESTS
