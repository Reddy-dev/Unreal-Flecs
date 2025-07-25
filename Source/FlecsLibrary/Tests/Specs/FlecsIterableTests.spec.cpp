﻿#if WITH_AUTOMATION_TESTS && defined(FLECS_TESTS)

#include "flecs.h"

#include "Misc/AutomationTest.h"
#include "Bake/FlecsTestUtils.h"
#include "Bake/FlecsTestTypes.h"

BEGIN_DEFINE_SPEC(FFlecsIterableTestsSpec,
                  "FlecsLibrary.Iterable",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

void Iterable_page_each(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    auto e1 = ecs.entity(); e1.set<Self>({e1});
    auto e2 = ecs.entity(); e2.set<Self>({e2});
    auto e3 = ecs.entity(); e3.set<Self>({e3});
    auto e4 = ecs.entity(); e4.set<Self>({e4});
    auto e5 = ecs.entity(); e5.set<Self>({e5});

    auto q = ecs.query<Self>();

    int32_t count = 0;
    q.page(1, 3).each([&](flecs::entity e, Self& self) {
        count ++;
        test_assert(e != e1);
        test_assert(e != e5);
        test_assert(e == self.value);
    });

    test_int(count, 3);
}

void Iterable_page_iter(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    auto e1 = ecs.entity(); e1.set<Self>({e1});
    auto e2 = ecs.entity(); e2.set<Self>({e2});
    auto e3 = ecs.entity(); e3.set<Self>({e3});
    auto e4 = ecs.entity(); e4.set<Self>({e4});
    auto e5 = ecs.entity(); e5.set<Self>({e5});

    auto q = ecs.query<Self>();

    int32_t count = 0;
    q.page(1, 3).run([&](flecs::iter it) {
        while (it.next()) {
            auto self = it.field<Self>(0);
            test_int(it.count(), 3);
            test_assert(it.entity(0) == e2);
            test_assert(it.entity(1) == e3);
            test_assert(it.entity(2) == e4);
            test_assert(it.entity(0) == self[0].value);
            test_assert(it.entity(1) == self[1].value);
            test_assert(it.entity(2) == self[2].value);
            count += it.count();
        }
    });

    test_int(count, 3);
}

void Iterable_worker_each(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    auto e1 = ecs.entity(); e1.set<Self>({e1});
    auto e2 = ecs.entity(); e2.set<Self>({e2});
    auto e3 = ecs.entity(); e3.set<Self>({e3});
    auto e4 = ecs.entity(); e4.set<Self>({e4});
    auto e5 = ecs.entity(); e5.set<Self>({e5});

    auto q = ecs.query<Self>();

    int32_t count = 0;
    q.worker(0, 2).each([&](flecs::entity e, Self& self) {
        count ++;
        test_assert(e != e4);
        test_assert(e != e5);
        test_assert(e == self.value);
    });

    test_int(count, 3);

    count = 0;
    q.worker(1, 2).each([&](flecs::entity e, Self& self) {
        count ++;
        test_assert(e != e1);
        test_assert(e != e2);
        test_assert(e != e3);
        test_assert(e == self.value);
    });

    test_int(count, 2);
}

void Iterable_worker_iter(void) {
    flecs::world ecs;
    RegisterTestTypeComponents(ecs);

    auto e1 = ecs.entity(); e1.set<Self>({e1});
    auto e2 = ecs.entity(); e2.set<Self>({e2});
    auto e3 = ecs.entity(); e3.set<Self>({e3});
    auto e4 = ecs.entity(); e4.set<Self>({e4});
    auto e5 = ecs.entity(); e5.set<Self>({e5});

    auto q = ecs.query<Self>();

    int32_t count = 0;
    q.worker(0, 2).run([&](flecs::iter it) {
        while (it.next()) {
            auto self = it.field<Self>(0);
            test_int(it.count(), 3);
            test_assert(it.entity(0) == e1);
            test_assert(it.entity(1) == e2);
            test_assert(it.entity(2) == e3);
            test_assert(it.entity(0) == self[0].value);
            test_assert(it.entity(1) == self[1].value);
            test_assert(it.entity(2) == self[2].value);
            count += it.count();
        }
    });

    test_int(count, 3);

    count = 0;
    q.worker(1, 2).run([&](flecs::iter it) {
        while (it.next()) {
            auto self = it.field<Self>(0);
            test_int(it.count(), 2);
            test_assert(it.entity(0) == e4);
            test_assert(it.entity(1) == e5);
            test_assert(it.entity(0) == self[0].value);
            test_assert(it.entity(1) == self[1].value);
            count += it.count();
        }
    });

    test_int(count, 2);
}


END_DEFINE_SPEC(FFlecsIterableTestsSpec);

/*"id": "Iterable",
"testcases": [
    "page_each",
    "page_iter",
    "worker_each",
    "worker_iter"
]*/

void FFlecsIterableTestsSpec::Define()
{
	It("Iterable_page_each", [&]() { Iterable_page_each(); });
    It("Iterable_page_iter", [&]() { Iterable_page_iter(); });
    It("Iterable_worker_each", [&]() { Iterable_worker_each(); });
    It("Iterable_worker_iter", [&]() { Iterable_worker_iter(); });
}

#endif // WITH_AUTOMATION_TESTS
