﻿#if WITH_AUTOMATION_TESTS && defined(FLECS_TESTS)

#include "flecs.h"

#include "Misc/AutomationTest.h"
#include "Bake/FlecsTestUtils.h"
#include "Bake/FlecsTestTypes.h"

BEGIN_DEFINE_SPEC(FFlecsSingletonTestsSpec,
                  "FlecsLibrary.Singleton",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

void Singleton_set_get_singleton() {
    flecs::world world;
    world.component<Position>();

    world.set<Position>({10, 20});

    const Position *p = world.try_get<Position>();
    test_assert(p != NULL);
    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_ensure_singleton() {
    flecs::world world;
    world.component<Position>();

    Position& p_mut = world.obtain<Position>();
    p_mut.x = 10;
    p_mut.y = 20;

    const Position *p = world.try_get<Position>();
    test_assert(p != NULL);
    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_get_mut_singleton() {
    flecs::world world;
    world.component<Position>();

    Position *p = world.try_get_mut<Position>();
    test_assert(p == nullptr);

    world.set<Position>({10, 20});
    p = world.try_get_mut<Position>();
    test_assert(p != nullptr);
    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_emplace_singleton() {
    flecs::world world;
    world.component<Position>();

    world.emplace<Position>(10.0f, 20.0f);

    const Position *p = world.try_get<Position>();
    test_assert(p != NULL);
    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_modified_singleton() {
    flecs::world world;
    world.component<Position>();

    int invoked = 0;

    world.observer<Position>()
        .event(flecs::OnSet)
        .each([&](Position&) {
            invoked ++;
        });

    auto e = world.entity();
    e.obtain<Position>();
    test_int(invoked, 0);

    e.modified<Position>();
    test_int(invoked, 1);
}

void Singleton_add_singleton() {
    flecs::world world;
    world.component<Position>();

    int invoked = 0;

    world.observer<Position>()
        .event(flecs::OnAdd)
        .each([&](Position& p) {
            invoked ++;
        });

    world.add<Position>();

    test_int(invoked, 1);
}

void Singleton_remove_singleton() {
    flecs::world world;
    world.component<Position>();

    int invoked = 0;

    world.observer<Position>()
        .event(flecs::OnRemove)
        .each([&](Position& p) {
            invoked ++;
        });

    world.obtain<Position>();
    test_int(invoked, 0);

    world.remove<Position>();
    test_int(invoked, 1);
}

void Singleton_has_singleton() {
    flecs::world world;
    world.component<Position>();

    test_assert(!world.has<Position>());

    world.set<Position>({10, 20});

    test_assert(world.has<Position>());
}

void Singleton_singleton_system() {
    flecs::world world;
    world.component<Position>();

    world.component<Position>().add(flecs::Singleton);

    world.set<Position>({10, 20});

    world.system<>()
        .expr("[inout] Position")
        .run([](flecs::iter& it) {
            while (it.next()) {
                auto p = it.field<Position>(0);
                test_int(p->x, 10);
                test_int(p->y, 20);

                p->x ++;
                p->y ++;
            }
        });

    world.progress();

    const Position *p = world.try_get<Position>();
    test_assert(p != NULL);
    test_int(p->x, 11);
    test_int(p->y, 21);
}

void Singleton_get_singleton() {
    flecs::world world;
    world.component<Position>();

    world.set<Position>({10, 20});

    auto s = world.singleton<Position>();
    test_assert(s.has<Position>());
    test_assert(s.id() == world.id<Position>());

    const Position* p = s.try_get<Position>();
    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_type_id_from_world() {
    flecs::world world;
    world.component<Position>();

    world.set<Position>({10, 20});

    flecs::entity_t id = world.id<Position>();
    test_assert(id == world.id<Position>());

    auto s = world.singleton<Position>();
    test_assert(s.id() == world.id<Position>());
    test_assert(s.id() == world.id<Position>());
}

void Singleton_set_lambda() {
    flecs::world world;
    world.component<Position>();

    world.set([](Position& p) {
        p.x = 10;
        p.y = 20;
    });

    const Position* p = world.try_get<Position>();
    test_int(p->x, 10);
    test_int(p->y, 20);

    world.set([](Position& p) {
        p.x ++;
        p.y ++;
    });

    p = world.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 21);
}

void Singleton_get_lambda() {
    flecs::world world;
    world.component<Position>();

    world.set<Position>({10, 20});

    int32_t count = 0;
    world.get([&](const Position& p) {
        test_int(p.x, 10);
        test_int(p.y, 20);
        count ++;
    });

    test_int(count, 1);
}

void Singleton_get_write_lambda() {
    flecs::world world;
    world.component<Position>();

    world.set<Position>({10, 20});

    int32_t count = 0;
    world.get([&](Position& p) {
        test_int(p.x, 10);
        test_int(p.y, 20);
        p.x ++;
        p.y ++;
        count ++;
    });

    test_int(count, 1);

    const Position *p = world.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 21);
}

void Singleton_get_set_singleton_pair_R_T() {
    flecs::world world;
    world.component<Position>();
    world.component<Tag>();

    world.set<Position, Tag>({10, 20});

    const Position *p = world.try_get<Position, Tag>();
    test_assert(p != nullptr);
    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_get_set_singleton_pair_R_t() {
    flecs::world world;
    world.component<Position>();

    flecs::entity tgt = world.entity();

    world.set<Position>(tgt, {10, 20});

    const Position *p = world.try_get<Position>(tgt);
    test_assert(p != nullptr);
    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_add_remove_singleton_pair_R_T() {
    flecs::world world;
    world.component<Position>();
    world.component<Tag>();

    world.add<Position, Tag>();
    test_assert((world.has<Position, Tag>()));
    world.remove<Position, Tag>();
    test_assert(!(world.has<Position, Tag>()));
}

void Singleton_add_remove_singleton_pair_R_t() {
    flecs::world world;
    world.component<Position>();

    flecs::entity tgt = world.entity();

    world.add<Position>(tgt);
    test_assert((world.has<Position>(tgt)));
    world.remove<Position>(tgt);
    test_assert(!(world.has<Position>(tgt)));
}

void Singleton_add_remove_singleton_pair_r_t() {
    flecs::world world;

    flecs::entity rel = world.entity();
    flecs::entity tgt = world.entity();

    world.add(rel, tgt);
    test_assert((world.has(rel, tgt)));
    world.remove(rel, tgt);
    test_assert(!(world.has(rel, tgt)));
}

void Singleton_get_target() {
    flecs::world world;
    world.component<Position>();
    world.component<Tag>();
    world.component<Velocity>();
    world.component<Mass>();

    auto Rel = world.singleton<Tag>();

    auto obj1 = world.entity()
        .add<Position>();

    auto obj2 = world.entity()
        .add<Velocity>();

    auto obj3 = world.entity()
        .add<Mass>();

    flecs::entity entities[3] = {obj1, obj2, obj3};

    world.add<Tag>(obj1);
    world.add<Tag>(obj2);
    world.add(Rel, obj3);

    auto p = world.target<Tag>();
    test_assert(p != 0);
    test_assert(p == obj1);

    p = world.target<Tag>(Rel);
    test_assert(p != 0);
    test_assert(p == obj1);

    p = world.target(Rel);
    test_assert(p != 0);
    test_assert(p == obj1);

    for (int i = 0; i < 3; i++) {
        p = world.target<Tag>(i);
        test_assert(p != 0);
        test_assert(p == entities[i]);
    }

    for (int i = 0; i < 3; i++) {
        p = world.target<Tag>(Rel, i);
        test_assert(p != 0);
        test_assert(p == entities[i]);
    }

    for (int i = 0; i < 3; i++) {
        p = world.target(Rel, i);
        test_assert(p != 0);
        test_assert(p == entities[i]);
    }
}

enum FlecsTestColor {
    Red, Green, Blue
};

void Singleton_singleton_enum(void) {
    flecs::world world;
    using Color = FlecsTestColor;
    world.component<Color>();

    world.set<Color>(Blue);
    test_assert(world.has<Color>());

    {
        const Color *c = world.try_get<Color>();
        test_assert(c != nullptr);
        test_assert(*c == Blue);
    }

    world.set<Color>(Green);
    test_assert(world.has<Color>());

    {
        const Color *c = world.try_get<Color>();
        test_assert(c != nullptr);
        test_assert(*c == Green);
    }

    world.remove<Color>();
    test_assert(!world.has<Color>());
}

void Singleton_get_w_id(void) {
    flecs::world world;
    world.component<Position>();

    world.set<Position>({10, 20});
    
    const Position *p = static_cast<const Position*>(world.get(world.id<Position>()));
    test_assert(p != nullptr);

    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_get_T(void) {
    flecs::world world;
    world.component<Position>();

    world.set<Position>({10, 20});
    
    const Position& p = world.get<Position>();
    test_int(p.x, 10);
    test_int(p.y, 20);
}

void Singleton_get_r_t(void) {
    flecs::world world;
    world.component<Position>();
    
    flecs::entity tgt = world.entity();

    world.set<Position>(tgt, {10, 20});
    
    const Position* p = static_cast<const Position*>(world.get(world.id<Position>(), tgt));
    test_assert(p != nullptr);

    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_get_R_t(void) {
    flecs::world world;
    world.component<Position>();

    flecs::entity tgt = world.entity();
    world.set<Position>(tgt, {10, 20});
    
    const Position& p = world.get<Position>(tgt);
    test_int(p.x, 10);
    test_int(p.y, 20);
}

void Singleton_get_R_T(void) {
    flecs::world world;
    world.component<Position>();

    struct Tgt { };
    world.component<Tgt>();

    world.set<Position, Tgt>({10, 20});
    
    const Position& p = world.get<Position, Tgt>();
    test_int(p.x, 10);
    test_int(p.y, 20);
}

void Singleton_try_get_w_id(void) {
    flecs::world world;
    world.component<Position>();

    const Position *p = static_cast<const Position*>(world.try_get(world.id<Position>()));
    test_assert(p == nullptr);

    world.set<Position>({10, 20});
    
    p = static_cast<const Position*>(world.try_get(world.id<Position>()));
    test_assert(p != nullptr);

    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_try_get_T(void) {
    flecs::world world;
    world.component<Position>();

    const Position *p = world.try_get<Position>();
    test_assert(p == nullptr);

    world.set<Position>({10, 20});
    
    p = world.try_get<Position>();
    test_assert(p != nullptr);

    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_try_get_r_t(void) {
    flecs::world world;
    world.component<Position>();

    flecs::entity tgt = world.entity();

    const Position *p = static_cast<const Position*>(world.try_get_mut(world.id<Position>(), tgt));
    test_assert(p == nullptr);

    world.set<Position>(tgt, {10, 20});
    
    p = static_cast<const Position*>(world.get_mut(world.id<Position>(), tgt));
    test_assert(p != nullptr);

    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_try_get_R_t(void) {
    flecs::world world;
    world.component<Position>();

    flecs::entity tgt = world.entity();

    const Position *p = world.try_get<Position>(tgt);
    test_assert(p == nullptr);

    world.set<Position>(tgt, {10, 20});
    
    p = world.try_get<Position>(tgt);
    test_assert(p != nullptr);

    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_try_get_R_T(void) {
    flecs::world world;
    world.component<Position>();

    struct Tgt { };
    world.component<Tgt>();

    const Position *p = world.try_get<Position, Tgt>();
    test_assert(p == nullptr);

    world.set<Position, Tgt>({10, 20});
    
    p = world.try_get<Position, Tgt>();
    test_assert(p != nullptr);

    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_get_mut_w_id(void) {
    flecs::world world;
    world.component<Position>();

    world.set<Position>({10, 20});
    
    Position *p = static_cast<Position*>(world.get_mut(world.id<Position>()));
    test_assert(p != nullptr);

    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_get_mut_T(void) {
    flecs::world world;
    world.component<Position>();

    world.set<Position>({10, 20});
    
    Position& p = world.get_mut<Position>();
    test_int(p.x, 10);
    test_int(p.y, 20);
}

void Singleton_get_mut_r_t(void) {
    flecs::world world;
    world.component<Position>();

    flecs::entity tgt = world.entity();

    world.set<Position>(tgt, {10, 20});
    
    Position* p = static_cast<Position*>(world.get_mut(world.id<Position>(), tgt));
    test_assert(p != nullptr);

    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_get_mut_R_t(void) {
    flecs::world world;
    world.component<Position>();

    flecs::entity tgt = world.entity();
    world.set<Position>(tgt, {10, 20});
    
    Position& p = world.get_mut<Position>(tgt);
    test_int(p.x, 10);
    test_int(p.y, 20);
}

void Singleton_get_mut_R_T(void) {
    flecs::world world;
    world.component<Position>();

    struct Tgt { };
    world.component<Tgt>();

    world.set<Position, Tgt>({10, 20});
    
    Position& p = world.get_mut<Position, Tgt>();
    test_int(p.x, 10);
    test_int(p.y, 20);
}

void Singleton_try_get_mut_w_id(void) {
    flecs::world world;
    world.component<Position>();
    
    Position *p = static_cast<Position*>(world.try_get_mut(world.id<Position>()));
    test_assert(p == nullptr);

    world.set<Position>({10, 20});
    
    p = static_cast<Position*>(world.try_get_mut(world.id<Position>()));
    test_assert(p != nullptr);

    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_try_get_mut_T(void) {
    flecs::world world;
    world.component<Position>();

    Position *p = world.try_get_mut<Position>();
    test_assert(p == nullptr);

    world.set<Position>({10, 20});
    
    p = world.try_get_mut<Position>();
    test_assert(p != nullptr);

    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_try_get_mut_r_t(void) {
    flecs::world world;
    world.component<Position>();

    flecs::entity tgt = world.entity();

    Position *p = static_cast<Position*>(world.try_get_mut(world.id<Position>(), tgt));
    test_assert(p == nullptr);

    world.set<Position>(tgt, {10, 20});
    
    p = static_cast<Position*>(world.get_mut(world.id<Position>(), tgt));
    test_assert(p != nullptr);

    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_try_get_mut_R_t(void) {
    flecs::world world;
    world.component<Position>();

    flecs::entity tgt = world.entity();

    Position *p = world.try_get_mut<Position>(tgt);
    test_assert(p == nullptr);

    world.set<Position>(tgt, {10, 20});
    
    p = world.try_get_mut<Position>(tgt);
    test_assert(p != nullptr);

    test_int(p->x, 10);
    test_int(p->y, 20);
}

void Singleton_try_get_mut_R_T(void) {
    flecs::world world;
    world.component<Position>();

    struct Tgt { };
    world.component<Tgt>();

    Position *p = world.try_get_mut<Position, Tgt>();
    test_assert(p == nullptr);

    world.set<Position, Tgt>({10, 20});
    
    p = world.try_get_mut<Position, Tgt>();
    test_assert(p != nullptr);

    test_int(p->x, 10);
    test_int(p->y, 20);
}

END_DEFINE_SPEC(FFlecsSingletonTestsSpec);

                /*"id": "Singleton",
                "testcases": [
                "set_get_singleton",
                "ensure_singleton",
                "get_mut_singleton",
                "emplace_singleton",
                "modified_singleton",
                "add_singleton",
                "remove_singleton",
                "has_singleton",
                "singleton_system",
                "get_singleton",
                "type_id_from_world",
                "set_lambda",
                "get_lambda",
                "get_write_lambda",
                "get_set_singleton_pair_R_T",
                "get_set_singleton_pair_R_t",
                "add_remove_singleton_pair_R_T",
                "add_remove_singleton_pair_R_t",
                "add_remove_singleton_pair_r_t",
                "get_target",
                "singleton_enum",
                "get_w_id",
                "get_T",
                "get_r_t",
                "get_R_t",
                "get_R_T",
                "get_w_id_not_found",
                "get_T_not_found",
                "get_r_t_not_found",
                "get_R_t_not_found",
                "get_R_T_not_found",
                "try_get_w_id",
                "try_get_T",
                "try_get_R_t",
                "try_get_R_T",
                "get_mut_w_id",
                "get_mut_T",
                "get_mut_R_t",
                "get_mut_R_T",
                "try_get_mut_w_id",
                "try_get_mut_T",
                "try_get_mut_r_t",
                "try_get_mut_R_t",
                "try_get_mut_R_T"
]*/

void FFlecsSingletonTestsSpec::Define()
{
	It("Singleton_set_get_singleton", [&]() { Singleton_set_get_singleton(); });
    It("Singleton_ensure_singleton", [&]() { Singleton_ensure_singleton(); });
    It("Singleton_get_mut_singleton", [&]() { Singleton_get_mut_singleton(); });
    It("Singleton_emplace_singleton", [&]() { Singleton_emplace_singleton(); });
    It("Singleton_modified_singleton", [&]() { Singleton_modified_singleton(); });
    It("Singleton_add_singleton", [&]() { Singleton_add_singleton(); });
    It("Singleton_remove_singleton", [&]() { Singleton_remove_singleton(); });
    It("Singleton_has_singleton", [&]() { Singleton_has_singleton(); });
    It("Singleton_singleton_system", [&]() { Singleton_singleton_system(); });
    It("Singleton_get_singleton", [&]() { Singleton_get_singleton(); });
    It("Singleton_type_id_from_world", [&]() { Singleton_type_id_from_world(); });
    It("Singleton_set_lambda", [&]() { Singleton_set_lambda(); });
    It("Singleton_get_lambda", [&]() { Singleton_get_lambda(); });
    It("Singleton_get_write_lambda", [&]() { Singleton_get_write_lambda(); });
    It("Singleton_get_set_singleton_pair_R_type_T_type", [&]() { Singleton_get_set_singleton_pair_R_T(); });
    It("Singleton_get_set_singleton_pair_R_type_t_entity", [&]() { Singleton_get_set_singleton_pair_R_t(); });
    It("Singleton_add_remove_singleton_pair_R_type_T_type", [&]() { Singleton_add_remove_singleton_pair_R_T(); });
    It("Singleton_add_remove_singleton_pair_R_type_t_entity", [&]() { Singleton_add_remove_singleton_pair_R_t(); });
    It("Singleton_add_remove_singleton_pair_r_entity_t_entity", [&]() { Singleton_add_remove_singleton_pair_r_t(); });
    It("Singleton_get_target", [&]() { Singleton_get_target(); });
    It("Singleton_singleton_enum", [&]() { Singleton_singleton_enum(); });
    It("Singleton_get_w_id", [&]() { Singleton_get_w_id(); });
    It("Singleton_get_T_type", [&]() { Singleton_get_T(); });
    It("Singleton_get_r_entity_t_entity", [&]() { Singleton_get_r_t(); });
    It("Singleton_get_R_type_t_entity", [&]() { Singleton_get_R_t(); });
    It("Singleton_get_R_type_T_type", [&]() { Singleton_get_R_T(); });
    It("Singleton_try_get_w_id", [&]() { Singleton_try_get_w_id(); });
    It("Singleton_try_get_T_type", [&]() { Singleton_try_get_T(); });
    It("Singleton_try_get_R_type_t_entity", [&]() { Singleton_try_get_R_t(); });
    It("Singleton_try_get_R_type_T_type", [&]() { Singleton_try_get_R_T(); });
    It("Singleton_get_mut_w_id", [&]() { Singleton_get_mut_w_id(); });
    It("Singleton_get_mut_T_type", [&]() { Singleton_get_mut_T(); });
    It("Singleton_get_mut_R_type_t_entity", [&]() { Singleton_get_mut_R_t(); });
    It("Singleton_get_mut_R_type_T_type", [&]() { Singleton_get_mut_R_T(); });
    It("Singleton_try_get_mut_w_id", [&]() { Singleton_try_get_mut_w_id(); });
    It("Singleton_try_get_mut_T_type", [&]() { Singleton_try_get_mut_T(); });
    It("Singleton_try_get_mut_r_entity_t_entity", [&]() { Singleton_try_get_mut_r_t(); });
    It("Singleton_try_get_mut_R_type_t_entity", [&]() { Singleton_try_get_mut_R_t(); });
    It("Singleton_try_get_mut_R_type_T_type", [&]() { Singleton_try_get_mut_R_T(); });
}
    

#endif // WITH_AUTOMATION_TESTS
