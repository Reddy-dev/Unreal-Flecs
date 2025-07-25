﻿#if WITH_AUTOMATION_TESTS && defined(FLECS_TESTS)

#include "flecs.h"

#include "Misc/AutomationTest.h"
#include "Bake/FlecsTestUtils.h"
#include "Bake/FlecsTestTypes.h"

struct MyTag { };

namespace ns {
	struct Velocity {
		float x, y;
	};
}

BEGIN_DEFINE_SPEC(FFlecsObserverTestsSpec,
                  "FlecsLibrary.Observer",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

void Observer_2_terms_on_add(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	int32_t count = 0;

	ecs.observer<Position, Velocity>()
		.event(flecs::OnAdd)
		.each([&](Position& p, Velocity& v) {
			count ++;          
		});

	auto e = ecs.entity();
	test_int(count, 0);

	e.set<Position>({10, 20});
	test_int(count, 0);

	e.set<Velocity>({1, 2});
	test_int(count, 1);
}

void Observer_2_terms_on_remove(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	int32_t count = 0;

	ecs.observer<Position, Velocity>()
		.event(flecs::OnRemove)
		.each([&](Position& p, Velocity& v) {
			count ++;
			test_int(p.x, 10);
			test_int(p.y, 20);
			test_int(v.x, 1);
			test_int(v.y, 2);
		});

	auto e = ecs.entity();
	test_int(count, 0);

	e.set<Position>({10, 20});
	test_int(count, 0);

	e.set<Velocity>({1, 2});
	test_int(count, 0);

	e.remove<Velocity>();
	test_int(count, 1);

	e.remove<Position>();
	test_int(count, 1);    
}

void Observer_2_terms_on_set(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	int32_t count = 0;

	ecs.observer<Position, Velocity>()
		.event(flecs::OnSet)
		.each([&](Position& p, Velocity& v) {
			count ++;
			test_int(p.x, 10);
			test_int(p.y, 20);
			test_int(v.x, 1);
			test_int(v.y, 2);            
		});

	auto e = ecs.entity();
	test_int(count, 0);

	e.set<Position>({10, 20});
	test_int(count, 0);

	e.set<Velocity>({1, 2});
	test_int(count, 1);
}


void Observer_2_terms_un_set(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	int32_t count = 0;

	ecs.observer<Position, Velocity>()
		.event(flecs::OnRemove)
		.each([&](Position& p, Velocity& v) {
			count ++;
			test_int(p.x, 10);
			test_int(p.y, 20);
			test_int(v.x, 1);
			test_int(v.y, 2);
		});

	auto e = ecs.entity();
	test_int(count, 0);

	e.set<Position>({10, 20});
	test_int(count, 0);

	e.set<Velocity>({1, 2});
	test_int(count, 0);

	e.remove<Velocity>();
	test_int(count, 1);

	e.remove<Position>();
	test_int(count, 1); 
}

void Observer_10_terms(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	int count = 0;

	auto e = ecs.entity();

	ecs.observer<>()
		.event(flecs::OnAdd)
		.with<Tag0>()
		.with<Tag1>()
		.with<Tag2>()
		.with<Tag3>()
		.with<Tag4>()
		.with<Tag5>()
		.with<Tag6>()
		.with<Tag7>()
		.with<Tag8>()
		.with<Tag9>()
		.each([&](flecs::iter& it, size_t) {
			test_int(it.count(), 1);
			test_assert(it.entity(0) == e);
			test_int(it.field_count(), 10);
			count ++;
		});

	e.add<Tag0>()
		.add<Tag1>()
		.add<Tag2>()
		.add<Tag3>()
		.add<Tag4>()
		.add<Tag5>()
		.add<Tag6>()
		.add<Tag7>()
		.add<Tag8>()
		.add<Tag9>();

	test_int(count, 1);
}

void Observer_16_terms(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	int count = 0;

	auto e = ecs.entity();

	ecs.observer<>()
		.event(flecs::OnAdd)
		.with<Tag0>()
		.with<Tag1>()
		.with<Tag2>()
		.with<Tag3>()
		.with<Tag4>()
		.with<Tag5>()
		.with<Tag6>()
		.with<Tag7>()
		.with<Tag8>()
		.with<Tag9>()
		.with<Tag10>()
		.with<Tag11>()
		.with<Tag12>()
		.with<Tag13>()
		.with<Tag14>()
		.with<Tag15>()
		.each([&](flecs::iter& it, size_t) {
			test_int(it.count(), 1);
			test_assert(it.entity(0) == e);
			test_int(it.field_count(), 16);
			count ++;
		});

	e.add<Tag0>()
		.add<Tag1>()
		.add<Tag2>()
		.add<Tag3>()
		.add<Tag4>()
		.add<Tag5>()
		.add<Tag6>()
		.add<Tag7>()
		.add<Tag8>()
		.add<Tag9>()
		.add<Tag10>()
		.add<Tag11>()
		.add<Tag12>()
		.add<Tag13>()
		.add<Tag14>()
		.add<Tag15>()
		.add<Tag16>()
		.add<Tag17>()
		.add<Tag18>()
		.add<Tag19>();

	test_int(count, 1);
}

void Observer_2_entities_iter(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto e1 = ecs.entity();
	auto e2 = ecs.entity();

	int32_t count = 0;
	flecs::entity last;

	ecs.observer<const Position>()
		.event(flecs::OnSet)
		.run([&](flecs::iter& it) {
			while (it.next()) {
				auto p = it.field<const Position>(0);

				for (auto i : it) {
					count ++;
					if (it.entity(i) == e1) {
						test_int(p[i].x, 10);
						test_int(p[i].y, 20);
					} else if (it.entity(i) == e2) {
						test_int(p[i].x, 30);
						test_int(p[i].y, 40);
					} else {
						test_assert(false);
					}

					last = it.entity(i);
				}
			}
		});

	e1.set<Position>({ 10, 20 });
	test_int(count, 1);
	test_assert(last == e1);

	e2.set<Position>({ 30, 40 }); 
	test_int(count, 2);
	test_assert(last == e2);
}

void Observer_2_entities_table_column(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto e1 = ecs.entity();
	auto e2 = ecs.entity();

	int32_t count = 0;
	flecs::entity last;

	ecs.observer<const Position>()
		.event(flecs::OnSet)
		.run([&](flecs::iter& it) {
			while (it.next()) {
				auto p = it.range().get<Position>();

				for (auto i : it) {
					count ++;
					if (it.entity(i) == e1) {
						test_int(p[i].x, 10);
						test_int(p[i].y, 20);
					} else if (it.entity(i) == e2) {
						test_int(p[i].x, 30);
						test_int(p[i].y, 40);
					} else {
						test_assert(false);
					}

					last = it.entity(i);
				}
			}
		});

	e1.set<Position>({ 10, 20 });
	test_int(count, 1);
	test_assert(last == e1);

	e2.set<Position>({ 30, 40 }); 
	test_int(count, 2);
	test_assert(last == e2);
}

void Observer_2_entities_each(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto e1 = ecs.entity();
	auto e2 = ecs.entity();

	int32_t count = 0;
	flecs::entity last;

	ecs.observer<const Position>()
		.event(flecs::OnSet)
		.each([&](flecs::entity e, const Position& p) {
			count ++;
			if (e == e1) {
				test_int(p.x, 10);
				test_int(p.y, 20);
			} else if (e == e2) {
				test_int(p.x, 30);
				test_int(p.y, 40);
			} else {
				test_assert(false);
			}

			last = e;
		});

	e1.set<Position>({ 10, 20 });
	test_int(count, 1);
	test_assert(last == e1);

	e2.set<Position>({ 30, 40 }); 
	test_int(count, 2);
	test_assert(last == e2);
}

void Observer_create_w_no_template_args(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto e1 = ecs.entity();

	int32_t count = 0;

	ecs.observer()
		.with<Position>()
		.event(flecs::OnAdd)
		.each([&](flecs::entity e) {
			test_assert(e == e1);
			count ++;          
		});

	e1.set<Position>({10, 20});
	test_int(count, 1);
}

void Observer_yield_existing(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	auto e1 = world.entity().add<Tag0>();
	auto e2 = world.entity().add<Tag0>();
	auto e3 = world.entity().add<Tag0>().add<Tag1>();

	int32_t count = 0;

	world.observer<Tag0>()
		.event(flecs::OnAdd)
		.yield_existing()
		.each([&](flecs::entity e, Tag0) {
			if (e == e1) count ++;
			if (e == e2) count += 2;
			if (e == e3) count += 3;
		});

	test_int(count, 6);
}

void Observer_yield_existing_2_terms(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	auto e1 = world.entity().add<Tag0>().add<Tag1>();
	auto e2 = world.entity().add<Tag0>().add<Tag1>();
	auto e3 = world.entity().add<Tag0>().add<Tag1>().add<Tag2>();
	world.entity().add<Tag0>();
	world.entity().add<Tag1>();

	int32_t count = 0;

	world.observer<Tag0, Tag1>()
		.event(flecs::OnAdd)
		.yield_existing()
		.each([&](flecs::entity e, Tag0, Tag1) {
			if (e == e1) count ++;
			if (e == e2) count += 2;
			if (e == e3) count += 3;
		});

	test_int(count, 6);
}

void Observer_yield_existing_on_create_flag(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);
	
	auto e1 = world.entity().add<Tag0>();
	auto e2 = world.entity().add<Tag0>();
	auto e3 = world.entity().add<Tag0>().add<Tag1>();

	int32_t count = 0;

	auto o = world.observer<Tag0>()
		.event(flecs::OnAdd)
		.event(flecs::OnRemove)
		.observer_flags(EcsObserverYieldOnCreate)
		.each([&](flecs::entity e, Tag0) {
			if (e == e1) count ++;
			if (e == e2) count += 2;
			if (e == e3) count += 3;
		});

	test_int(count, 6);

	o.destruct();

	test_int(count, 6);
}

void Observer_yield_existing_on_delete_flag(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	auto e1 = world.entity().add<Tag0>();
	auto e2 = world.entity().add<Tag0>();
	auto e3 = world.entity().add<Tag0>().add<Tag1>();

	int32_t count = 0;

	auto o = world.observer<Tag0>()
		.event(flecs::OnAdd)
		.event(flecs::OnRemove)
		.observer_flags(EcsObserverYieldOnDelete)
		.each([&](flecs::entity e, Tag0) {
			if (e == e1) count ++;
			if (e == e2) count += 2;
			if (e == e3) count += 3;
		});

	test_int(count, 0);

	o.destruct();

	test_int(count, 6);
}

void Observer_yield_existing_on_create_delete_flag(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	auto e1 = world.entity().add<Tag0>();
	auto e2 = world.entity().add<Tag0>();
	auto e3 = world.entity().add<Tag0>().add<Tag1>();

	int32_t count = 0;

	auto o = world.observer<Tag0>()
		.event(flecs::OnAdd)
		.event(flecs::OnRemove)
		.observer_flags(EcsObserverYieldOnCreate|EcsObserverYieldOnDelete)
		.each([&](flecs::entity e, Tag0) {
			if (e == e1) count ++;
			if (e == e2) count += 2;
			if (e == e3) count += 3;
		});

	test_int(count, 6);

	o.destruct();

	test_int(count, 12);
}

void Observer_default_ctor(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	flecs::observer o;
	test_assert(o == 0);

	int32_t count = 0;
	o = world.observer<Tag0>()
		.event(flecs::OnAdd)
		.each([&](flecs::entity e, Tag0) {
			count ++;
		});
    
	world.entity().add<Tag0>();
    
	test_int(count, 1);
}

void Observer_entity_ctor(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	flecs::observer o = world.observer<Tag0>()
		.event(flecs::OnAdd)
		.each([&](flecs::entity e, Tag0) { });
    
	flecs::entity oe = o;

	flecs::observer eo = world.observer(oe);
	test_assert(eo == o);
}

void Observer_on_add(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	int invoked = 0;

	world.observer<Position>()
		.event(flecs::OnAdd)
		.each([&](flecs::entity e, Position& p) {
			invoked ++;
		});

	world.entity()
		.add<Position>();

	test_int(invoked, 1);
}

void Observer_on_remove(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	int invoked = 0;

	world.observer<Position>()
		.event(flecs::OnRemove)
		.each([&](flecs::entity e, Position& p) {
			invoked ++;
		});

	auto e = world.entity()
		.add<Position>();

	test_int(invoked, 0);
    
	e.remove<Position>();

	test_int(invoked, 1);
}

void Observer_on_add_tag_action(void) {
	flecs::world world;
	world.component<MyTag>();

	int invoked = 0;

	world.observer<MyTag>()
		.event(flecs::OnAdd)
		.run([&](flecs::iter it) {
			while (it.next()) {
				invoked ++;
			}
		});

	world.entity()
		.add<MyTag>();

	test_int(invoked, 1);
}

void Observer_on_add_tag_iter(void) {
	flecs::world world;
	world.component<MyTag>();

	int invoked = 0;

	world.observer<MyTag>()
		.event(flecs::OnAdd)
		.run([&](flecs::iter it) {
			while (it.next()) {
				invoked ++;
			}
		});

	world.entity()
		.add<MyTag>();

	test_int(invoked, 1);
}

void Observer_on_add_tag_each(void) {
	flecs::world world;
	world.component<MyTag>();

	int invoked = 0;

	world.observer<MyTag>()
		.event(flecs::OnAdd)
		.each([&](flecs::entity e, MyTag) {
			invoked ++;
		});

	world.entity()
		.add<MyTag>();

	test_int(invoked, 1);
}

void Observer_on_add_expr(void) {
	flecs::world world;

	int invoked = 0;

	world.component<Tag>();

	world.observer<>().expr("Tag")
		.event(flecs::OnAdd)
		.each([&](flecs::entity e) {
			invoked ++;
		});

	auto e = world.entity().add<Tag>();

	test_int(invoked, 1);
    
	e.remove<Tag>();

	test_int(invoked, 1);
}

void Observer_observer_w_filter_term(void) {
	flecs::world world;

	flecs::entity Tag0 = world.entity();
	flecs::entity Tag1 = world.entity();

	int invoked = 0;

	world.observer()
		.with(Tag0)
		.with(Tag1).filter()
		.event(flecs::OnAdd)
		.each([&](flecs::entity e) {
			invoked ++;
		});

	flecs::entity e = world.entity();
	test_int(invoked, 0);

	e.add(Tag1);
	test_int(invoked, 0);

	e.add(Tag0);
	test_int(invoked, 1);

	e.remove(Tag1);
	test_int(invoked, 1);

	e.add(Tag1);
	test_int(invoked, 1);

	e.clear();
	test_int(invoked, 1);

	e.add(Tag0);
	test_int(invoked, 1);
}

void Observer_run_callback(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	int32_t count = 0;

	ecs.observer<Position>()
		.event(flecs::OnAdd)
		.run([](flecs::iter& it) {
			while (it.next()) {
				it.each();
			}
		},
		[&](Position& p) {
			count ++;          
		});

	auto e = ecs.entity();
	test_int(count, 0);

	e.set<Position>({10, 20});
	test_int(count, 1);
}

void Observer_run_callback_w_1_field(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	int32_t count = 0;

	ecs.observer<Position>()
		.event(flecs::OnSet)
		.run([&](flecs::iter& it) {
			while (it.next()) {
				auto p = it.field<Position>(0);
				test_int(p->x, 10);
				test_int(p->y, 20);
                
				count ++;
			}
		});

	auto e = ecs.entity();
	test_int(count, 0);

	e.set<Position>({10, 20});
	test_int(count, 1);
}

void Observer_run_callback_w_2_fields(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	int32_t count = 0;

	ecs.observer<Position, Velocity>()
		.event(flecs::OnSet)
		.run([&](flecs::iter& it) {
			while (it.next()) {
				auto p = it.field<Position>(0);
				auto v = it.field<Velocity>(1);

				test_int(p->x, 10);
				test_int(p->y, 20);
				test_int(v->x, 1);
				test_int(v->y, 2);

				count ++;
			}
		});

	auto e = ecs.entity();
	test_int(count, 0);

	e.set<Position>({10, 20});
	test_int(count, 0);

	e.set<Velocity>({1, 2});
	test_int(count, 1);
}

void Observer_run_callback_w_yield_existing_1_field(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	int32_t count = 0;
	ecs.entity().set<Position>({10, 20});

	ecs.observer<Position>()
		.event(flecs::OnSet)
		.yield_existing()
		.run([&](flecs::iter& it) {
			while (it.next()) {
				auto p = it.field<Position>(0);

				test_int(p->x, 10);
				test_int(p->y, 20);

				count ++;
			}
		});
    
	test_int(count, 1);
}

void Observer_run_callback_w_yield_existing_2_fields(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	int32_t count = 0;
	ecs.entity().set<Position>({10, 20}).set<Velocity>({1, 2});

	ecs.observer<Position, Velocity>()
		.event(flecs::OnSet)
		.yield_existing()
		.run([&](flecs::iter& it) {
			while (it.next()) {
				auto p = it.field<Position>(0);
				auto v = it.field<Velocity>(1);

				test_int(p->x, 10);
				test_int(p->y, 20);
				test_int(v->x, 1);
				test_int(v->y, 2);

				count ++;
			}
		});
    
	test_int(count, 1);
}

void Observer_get_query(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	world.entity().set<Position>({0, 0});
	world.entity().set<Position>({1, 0});
	world.entity().set<Position>({2, 0});

	int32_t count = 0;

	auto sys = world.observer<const Position>()
		.event(flecs::OnSet)
		.each([&](flecs::entity e, const Position& p) {
			// Not used
		});

	auto q = sys.query();

	q.run([&](flecs::iter& it) {
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

void Observer_on_set_w_set(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	int32_t count = 0;

	world.observer<Position>()
		.event(flecs::OnSet)
		.each([&](flecs::entity e, Position& p) {
			count ++;          
		});

	flecs::entity e = world.entity();
	test_int(count, 0);

	e.set<Position>({10, 20});
	test_int(count, 1);
}

void Observer_on_set_w_defer_set(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	int32_t count = 0;

	world.observer<Position>()
		.event(flecs::OnSet)
		.each([&](flecs::entity e, Position& p) {
			count ++;          
		});

	flecs::entity e = world.entity();
	test_int(count, 0);

	world.defer_begin();
	e.set<Position>({10, 20});

	test_int(count, 0);
	world.defer_end();

	test_int(count, 1);
}


void Observer_on_set_w_set_sparse(void) {
	flecs::world world;

	world.component<Position>().add(flecs::Sparse);

	int32_t count = 0;

	world.observer<Position>()
		.event(flecs::OnSet)
		.each([&](flecs::entity e, Position& p) {
			count ++;          
		});

	flecs::entity e = world.entity();
	test_int(count, 0);

	e.set<Position>({10, 20});
	test_int(count, 1);
}

void Observer_on_add_singleton(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	world.component<Position>().add(flecs::Singleton);

	int32_t count = 0;

	world.observer<Position>()
		.term_at(0)
		.event(flecs::OnSet)
		.each([&](Position& p) {
			test_int(p.x, 10);
			test_int(p.y, 20);
			count ++;
		});

	world.set<Position>({10, 20});

	test_int(count, 1);
}

void Observer_on_add_pair_singleton(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	world.component<Position>().add(flecs::Singleton);

	int32_t count = 0;

	flecs::entity tgt = world.entity();

	world.observer<Position>()
		.term_at(0).second(tgt)
		.event(flecs::OnSet)
		.each([&](Position& p) {
			test_int(p.x, 10);
			test_int(p.y, 20);
			count ++;
		});

	world.set<Position>(tgt, {10, 20});

	test_int(count, 1);
}

void Observer_on_add_pair_wildcard_singleton(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	world.component<Position>().add(flecs::Singleton);

	int32_t count = 0;

	flecs::entity tgt_1 = world.entity();
	flecs::entity tgt_2 = world.entity();

	world.observer<Position>()
		.term_at(0).second(flecs::Wildcard)
		.event(flecs::OnSet)
		.each([&](Position& p) {
			test_int(p.x, 10);
			test_int(p.y, 20);
			count ++;
		});

	world.set<Position>(tgt_1, {10, 20});
	test_int(count, 1);

	world.set<Position>(tgt_2, {10, 20});
	test_int(count, 2);
}

void Observer_on_add_with_pair_singleton(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	world.component<Position>().add(flecs::Singleton);

	int32_t count = 0;

	flecs::entity tgt = world.entity();

	world.observer()
		.with<Position>(tgt)
		.event(flecs::OnSet)
		.each([&](flecs::iter&, size_t) {
			count ++;
		});

	world.set<Position>(tgt, {10, 20});
	test_int(count, 1);
}

void Observer_add_in_yield_existing(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	flecs::entity e1 = world.entity().set<Position>({});
	flecs::entity e2 = world.entity().set<Position>({});
	flecs::entity e3 = world.entity().set<Position>({});

	world.observer()
		.with<Position>()
		.event(flecs::OnAdd)
		.yield_existing()
		.each([](flecs::entity e) {
			e.add<Velocity>();
		});

	test_assert(e1.has<Position>());
	test_assert(e1.has<Velocity>());

	test_assert(e2.has<Position>());
	test_assert(e2.has<Velocity>());

	test_assert(e3.has<Position>());
	test_assert(e3.has<Velocity>());
}

void Observer_add_in_yield_existing_multi(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	flecs::entity e1 = world.entity().set<Position>({}).set<Mass>({});
	flecs::entity e2 = world.entity().set<Position>({}).set<Mass>({});
	flecs::entity e3 = world.entity().set<Position>({}).set<Mass>({});

	world.observer()
		.with<Position>()
		.with<Mass>()
		.event(flecs::OnAdd)
		.yield_existing()
		.each([](flecs::entity e) {
			e.add<Velocity>();
		});

	test_assert(e1.has<Position>());
	test_assert(e1.has<Mass>());
	test_assert(e1.has<Velocity>());

	test_assert(e2.has<Position>());
	test_assert(e2.has<Mass>());
	test_assert(e2.has<Velocity>());

	test_assert(e3.has<Position>());
	test_assert(e3.has<Mass>());
	test_assert(e3.has<Velocity>());
}

void Observer_name_from_root(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	flecs::entity sys = ecs.observer<Position>("::ns::MySystem")
		.event(flecs::OnSet)
		.each([](Position& p) { });

	test_str(sys.name(), "MySystem");

	flecs::entity ns = ecs.entity("::ns");
	test_assert(ns == sys.parent());
}

void Observer_term_index(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto e1 = ecs.entity().add<Position>().add<Velocity>();

	int32_t last_term = -1;

	ecs.observer<const Position, const Velocity>()
		.event(flecs::OnSet)
		.each([&](flecs::iter& it, size_t row, const Position &p, const Velocity &v) {
			last_term = it.term_index();
		});

	e1.set<Position>({ 10, 20 });
	test_int(last_term, 0);

	e1.set<Velocity>({ 30, 40 }); 
	test_int(last_term, 1);
}

void Observer_implicit_register_in_emit_for_named_entity(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	struct MyEvent { float value; };
	world.component<MyEvent>();

	flecs::entity e1 = world.entity("e1");
	flecs::entity e2 = world.entity();

	e1.observe<MyEvent>([&](MyEvent& evt) {
		test_int(evt.value, 10);
		e2.set<Position>({10, 20});
	});

	e1.emit<MyEvent>({ 10 });
}

void Observer_add_to_named_in_emit_for_named_entity(void) {
	flecs::world world;

	world.component<Position>();

	struct MyEvent { float value; };
	world.component<MyEvent>();

	flecs::entity e1 = world.entity("e1");
	flecs::entity e2 = world.entity("e2");

	e1.observe<MyEvent>([&](MyEvent& evt) {
		test_int(evt.value, 10);
		e2.set<Position>({10, 20});
	});

	e1.emit<MyEvent>({ 10 });
}

void Observer_implicit_register_in_emit_for_named_entity_w_defer(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	struct MyEvent { float value; };
	world.component<MyEvent>();

	flecs::entity e1 = world.entity("e1");
	flecs::entity e2 = world.entity();

	e1.observe<MyEvent>([&](MyEvent& evt) {
		test_int(evt.value, 10);
		e2.set<Position>({10, 20});
	});

	world.defer_begin();
	e1.emit<MyEvent>({ 10 });
	world.defer_end();
}

void Observer_add_to_named_in_emit_for_named_entity_w_defer(void) {
	flecs::world world;

	world.component<Position>();

	struct MyEvent { float value; };
	world.component<MyEvent>();

	flecs::entity e1 = world.entity("e1");
	flecs::entity e2 = world.entity("e2");

	e1.observe<MyEvent>([&](MyEvent& evt) {
		test_int(evt.value, 10);
		e2.set<Position>({10, 20});
	});

	world.defer_begin();
	e1.emit<MyEvent>({ 10 });
	world.defer_end();
}

void Observer_register_twice_w_each(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	int count1 = 0, count2 = 0;

	ecs.observer<Position>("Test")
		.event(flecs::OnSet)
		.each([&](Position&) {
			count1 ++;
		});

	ecs.entity().set(Position{10, 20});
	test_int(count1, 1);

	ecs.observer<Position>("Test")
		.event(flecs::OnSet)
		.each([&](Position&) {
			count2 ++;
		});

	ecs.entity().set(Position{10, 20});
	test_int(count2, 1);
}

void Observer_register_twice_w_run(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	int count1 = 0, count2 = 0;

	ecs.observer<Position>("Test")
		.event(flecs::OnSet)
		.run([&](flecs::iter&) {
			count1 ++;
		});

	ecs.entity().set(Position{10, 20});
	test_int(count1, 1);

	ecs.observer<Position>("Test")
		.event(flecs::OnSet)
		.run([&](flecs::iter&) {
			count2 ++;
		});

	ecs.entity().set(Position{10, 20});
	test_int(count2, 1);
}


void Observer_register_twice_w_run_each(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	int count1 = 0, count2 = 0;

	ecs.observer<Position>("Test")
		.event(flecs::OnSet)
		.run([&](flecs::iter&) {
			count1 ++;
		});

	ecs.entity().set(Position{10, 20});
	test_int(count1, 1);

	ecs.observer<Position>("Test")
		.event(flecs::OnSet)
		.each([&](Position&) {
			count2 ++;
		});

	ecs.entity().set(Position{10, 20});
	test_int(count2, 1);
}

void Observer_register_twice_w_each_run(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	int count1 = 0, count2 = 0;

	ecs.observer<Position>("Test")
		.event(flecs::OnSet)
		.each([&](Position&) {
			count1 ++;
		});

	ecs.entity().set(Position{10, 20});
	test_int(count1, 1);

	ecs.observer<Position>("Test")
		.event(flecs::OnSet)
		.run([&](flecs::iter&) {
			count2 ++;
		});

	ecs.entity().set(Position{10, 20});
	test_int(count2, 1);
}

void Observer_other_table(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	int32_t count = 0;
	flecs::entity matched;

	ecs.observer<Velocity>()
		.event(flecs::OnAdd)
		.each([&](flecs::iter& it, size_t row, Velocity&) {
			test_assert(it.table().has<Velocity>());
			test_assert(!it.other_table().has<Velocity>());
			matched = it.entity(row);
			count ++;
		});

	flecs::entity e = ecs.entity().add<Position>().add<Velocity>();
	test_assert(e == matched);
	test_int(count, 1);
}

void Observer_other_table_w_pair(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	struct Likes {};
	struct Apples {};
	ecs.component<Likes>();
	ecs.component<Apples>();

	int32_t count = 0;
	flecs::entity matched;

	ecs.observer()
		.with<Likes, Apples>()
		.event(flecs::OnAdd)
		.each([&](flecs::iter& it, size_t row) {
			test_assert((it.table().has<Likes, Apples>()));
			test_assert((!it.other_table().has<Likes, Apples>()));
			matched = it.entity(row);
			count ++;
		});

	flecs::entity e = ecs.entity().add<Position>().add<Likes, Apples>();
	test_assert(e == matched);
	test_int(count, 1);
}

void Observer_other_table_w_pair_wildcard(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	struct Likes {};
	struct Apples {};
	ecs.component<Likes>();
	ecs.component<Apples>();

	int32_t count = 0;
	flecs::entity matched;

	ecs.observer()
		.with<Likes, Apples>()
		.event(flecs::OnAdd)
		.each([&](flecs::iter& it, size_t row) {
			test_assert((it.table().has<Likes>(flecs::Wildcard)));
			test_assert((!it.other_table().has<Likes>(flecs::Wildcard)));
			matched = it.entity(row);
			count ++;
		});

	flecs::entity e = ecs.entity().add<Position>().add<Likes, Apples>();
	test_assert(e == matched);
	test_int(count, 1);
}

void Observer_on_add_inherited(void) {
	flecs::world ecs;

	ecs.component<Position>().add(flecs::OnInstantiate, flecs::Inherit);

	int32_t count = 0;
	flecs::entity matched;

	ecs.observer<Position>()
		.event(flecs::OnAdd)
		.each([&](flecs::entity e, Position& p) {
			test_int(p.x, 10);
			test_int(p.y, 20);
			count ++;
			matched = e;
		});

	auto p = ecs.prefab().set(Position{10, 20});
	test_int(count, 0);

	auto i = ecs.entity().is_a(p);
	test_int(count, 1);
	test_assert(i == matched);
}

void Observer_on_set_inherited(void) {
	flecs::world ecs;

	ecs.component<Position>().add(flecs::OnInstantiate, flecs::Inherit);

	int32_t count = 0;
	flecs::entity matched;

	ecs.observer<Position>()
		.event(flecs::OnSet)
		.each([&](flecs::entity e, Position& p) {
			test_int(p.x, 10);
			test_int(p.y, 20);
			count ++;
			matched = e;
		});

	auto p = ecs.prefab().set(Position{10, 20});
	test_int(count, 0);

	auto i = ecs.entity().is_a(p);
	test_int(count, 1);
	test_assert(i == matched);
}

void Observer_on_remove_inherited(void) {
	flecs::world ecs;

	ecs.component<Position>().add(flecs::OnInstantiate, flecs::Inherit);

	int32_t count = 0;
	flecs::entity matched;

	ecs.observer<Position>()
		.event(flecs::OnRemove)
		.each([&](flecs::entity e, Position& p) {
			test_int(p.x, 10);
			test_int(p.y, 20);
			count ++;
			matched = e;
		});

	auto p = ecs.prefab().set(Position{10, 20});
	test_int(count, 0);

	auto i = ecs.entity().is_a(p);
	test_int(count, 0);

	p.remove<Position>();
	test_int(count, 1);
	test_assert(i == matched);
}


void Observer_on_set_after_remove_override(void) {
	flecs::world ecs;

	ecs.component<Position>().add(flecs::OnInstantiate, flecs::Inherit);

	auto base = ecs.prefab()
		.set(Position{1, 2});

	auto e1 = ecs.entity().is_a(base)
		.set(Position{10, 20});

	int count = 0;

	ecs.observer<Position>()
		.event(flecs::OnSet)
		.each([&](flecs::iter& it, size_t row, Position& p) {
			test_assert(it.entity(row) == e1);
			test_assert(it.src(0) == base);
			test_int(p.x, 1);
			test_int(p.y, 2);
			count ++;
		});

	e1.remove<Position>();

	test_int(count, 1);
}

void Observer_on_set_after_remove_override_create_observer_before(void) {
	flecs::world ecs;

	ecs.component<Position>().add(flecs::OnInstantiate, flecs::Inherit);

	int count = 0;

	flecs::entity base = ecs.prefab();
	flecs::entity e1 = ecs.entity();

	ecs.observer<Position>()
		.event(flecs::OnSet)
		.each([&](flecs::iter& it, size_t row, Position& p) {
			test_assert(it.entity(row) == e1);
			test_assert(it.src(0) == base);
			count ++;
		});

	base.set(Position{1, 2});
	e1.add<Position>().is_a(base);

	test_int(count, 0);

	e1.remove<Position>();

	test_int(count, 1);
}

void Observer_on_set_w_override_after_delete(void) {
	flecs::world ecs;

	ecs.component<Position>().add(flecs::OnInstantiate, flecs::Inherit);

	auto base = ecs.prefab()
		.set(Position{1, 2});

	auto e1 = ecs.entity().is_a(base)
		.set(Position{10, 20});

	int count = 0;

	ecs.observer<Position>()
		.event(flecs::OnSet)
		.each([&](flecs::iter& it, size_t row, Position& p) {
			count ++;
		});

	e1.destruct();

	test_int(count, 0);
}

void Observer_on_set_w_override_after_clear(void) {
	flecs::world ecs;

	ecs.component<Position>().add(flecs::OnInstantiate, flecs::Inherit);

	auto base = ecs.prefab()
		.set(Position{1, 2});

	auto e1 = ecs.entity().is_a(base)
		.set(Position{10, 20});

	int count = 0;

	ecs.observer<Position>()
		.event(flecs::OnSet)
		.each([&](flecs::iter& it, size_t row, Position& p) {
			count ++;
		});

	e1.clear();

	test_int(count, 0);
}

void Observer_trigger_on_set_in_on_add_implicit_registration(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);
  
	world.observer<Tag>()
	  .event(flecs::OnAdd)
	  .each([](flecs::entity e, Tag) {
		  e.set<Position>({ 10, 20 });
	  });

	world.observer<Position>()
	  .event(flecs::OnSet)
	  .each([](flecs::entity e, Position&) {
		  e.set<Velocity>({ 1, 2 });
	  });

	auto e = world.entity().add<Tag>();

	{
		const Position *p = e.try_get<Position>();
		test_int(p->x, 10);
		test_int(p->y, 20);
	}

	{
		const Velocity *v = e.try_get<Velocity>();
		test_int(v->x, 1);
		test_int(v->y, 2);
	}
}

void Observer_trigger_on_set_in_on_add_implicit_registration_namespaced(void) {
	flecs::world world;
	world.component<Position>();
	world.component<ns::Velocity>();
	world.component<Tag>();
    
	world.observer<Tag>()
		.event(flecs::OnAdd)
		.each([](flecs::entity e, Tag) {
			e.set<Position>({ 10, 20 });
		});

	world.observer<Position>()
		.event(flecs::OnSet)
		.each([](flecs::entity e, Position&) {
			e.set<ns::Velocity>({ 1, 2 });
		});

	auto e = world.entity().add<Tag>();

	{
		const Position *p = e.try_get<Position>();
		test_int(p->x, 10);
		test_int(p->y, 20);
	}

	{
		const ns::Velocity *v = e.try_get<ns::Velocity>();
		test_int(v->x, 1);
		test_int(v->y, 2);
	}
}


void Observer_fixed_src_w_each(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	flecs::entity matched;
	flecs::entity e = world.entity();

	world.observer()
		.with<Tag>().src(e)
		.event(flecs::OnAdd)
		.each([&](flecs::iter& it, size_t) {
			matched = it.src(0);
		});

	test_assert(matched == 0);

	e.add<Tag>();

	test_assert(matched == e);

	matched = flecs::entity::null();

	world.entity().add<Tag>();
    
	test_assert(matched == 0);
}


void Observer_fixed_src_w_run(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	flecs::entity matched;
	flecs::entity e = world.entity();

	world.observer()
		.with<Tag>().src(e)
		.event(flecs::OnAdd)
		.run([&](flecs::iter& it) {
			while (it.next()) {
				test_int(it.count(), 0);
				matched = it.src(0);
			}
		});

	test_assert(matched == 0);

	e.add<Tag>();

	test_assert(matched == e);

	matched = flecs::entity::null();

	world.entity().add<Tag>();
    
	test_assert(matched == 0);
}

END_DEFINE_SPEC(FFlecsObserverTestsSpec);

/*"id": "Observer",
            "testcases": [
                "2_terms_on_add",
                "2_terms_on_remove",
                "2_terms_on_set",
                "2_terms_un_set",
                "10_terms",
                "16_terms",
                "2_entities_iter",
                "2_entities_table_column",
                "2_entities_each",
                "create_w_no_template_args",
                "yield_existing",
                "yield_existing_2_terms",
                "yield_existing_on_create_flag",
                "yield_existing_on_delete_flag",
                "yield_existing_on_create_delete_flag",
                "default_ctor",
                "entity_ctor",
                "on_add",
                "on_remove",
                "on_add_tag_action",
                "on_add_tag_iter",
                "on_add_tag_each",
                "on_add_expr",
                "observer_w_filter_term",
                "run_callback",
                "run_callback_w_1_field",
                "run_callback_w_2_fields",
                "run_callback_w_yield_existing_1_field",
                "run_callback_w_yield_existing_2_fields",
                "get_query",
                "on_set_w_set",
                "on_set_w_defer_set",
                "on_set_w_set_sparse",
                "on_add_singleton",
                "on_add_pair_singleton",
                "on_add_pair_wildcard_singleton",
                "on_add_with_pair_singleton",
                "add_in_yield_existing",
                "add_in_yield_existing_multi",
                "name_from_root",
                "term_index",
                "implicit_register_in_emit_for_named_entity",
                "add_to_named_in_emit_for_named_entity",
                "implicit_register_in_emit_for_named_entity_w_defer",
                "add_to_named_in_emit_for_named_entity_w_defer",
                "register_twice_w_each",
                "register_twice_w_run",
                "register_twice_w_run_each",
                "register_twice_w_each_run",
                "other_table",
                "other_table_w_pair",
                "other_table_w_pair_wildcard",
                "on_add_inherited",
                "on_set_inherited",
                "on_remove_inherited",
                "on_set_after_remove_override",
                "on_set_after_remove_override_create_observer_before",
                "on_set_w_override_after_delete",
                "on_set_w_override_after_clear",
                "trigger_on_set_in_on_add_implicit_registration",
                "trigger_on_set_in_on_add_implicit_registration_namespaced",
                "fixed_src_w_each",
                "fixed_src_w_run"
            ]*/

void FFlecsObserverTestsSpec::Define()
{
	It("2_terms_on_add", [&]() { Observer_2_terms_on_add(); });
	It("2_terms_on_remove", [&]() { Observer_2_terms_on_remove(); });
	It("2_terms_on_set", [&]() { Observer_2_terms_on_set(); });
	It("2_terms_un_set", [&]() { Observer_2_terms_un_set(); });
	It("10_terms", [&]() { Observer_10_terms(); });
	It("16_terms", [&]() { Observer_16_terms(); });
	It("2_entities_iter", [&]() { Observer_2_entities_iter(); });
	It("2_entities_table_column", [&]() { Observer_2_entities_table_column(); });
	It("2_entities_each", [&]() { Observer_2_entities_each(); });
	It("create_w_no_template_args", [&]() { Observer_create_w_no_template_args(); });
	It("yield_existing", [&]() { Observer_yield_existing(); });
	It("yield_existing_2_terms", [&]() { Observer_yield_existing_2_terms(); });
	It("yield_existing_on_create_flag", [&]() { Observer_yield_existing_on_create_flag(); });
	It("yield_existing_on_delete_flag", [&]() { Observer_yield_existing_on_delete_flag(); });
	It("yield_existing_on_create_delete_flag", [&]() { Observer_yield_existing_on_create_delete_flag(); });
	It("default_ctor", [&]() { Observer_default_ctor(); });
	It("entity_ctor", [&]() { Observer_entity_ctor(); });
	It("on_add", [&]() { Observer_on_add(); });
	It("on_remove", [&]() { Observer_on_remove(); });
	It("on_add_tag_action", [&]() { Observer_on_add_tag_action(); });
	It("on_add_tag_iter", [&]() { Observer_on_add_tag_iter(); });
	It("on_add_tag_each", [&]() { Observer_on_add_tag_each(); });
	It("on_add_expr", [&]() { Observer_on_add_expr(); });
	It("observer_w_filter_term", [&]() { Observer_observer_w_filter_term(); });
	It("run_callback", [&]() { Observer_run_callback(); });
	It("run_callback_w_1_field", [&]() { Observer_run_callback_w_1_field(); });
	It("run_callback_w_2_fields", [&]() { Observer_run_callback_w_2_fields(); });
	It("run_callback_w_yield_existing_1_field", [&]() { Observer_run_callback_w_yield_existing_1_field(); });
	It("run_callback_w_yield_existing_2_fields", [&]() { Observer_run_callback_w_yield_existing_2_fields(); });
	It("get_query", [&]() { Observer_get_query(); });
	It("on_set_w_set", [&]() { Observer_on_set_w_set(); });
	It("on_set_w_defer_set", [&]() { Observer_on_set_w_defer_set(); });
	It("on_set_w_set_sparse", [&]() { Observer_on_set_w_set_sparse(); });
	It("on_add_singleton", [&]() { Observer_on_add_singleton(); });
	It("on_add_pair_singleton", [&]() { Observer_on_add_pair_singleton(); });
	It("on_add_pair_wildcard_singleton", [&]() { Observer_on_add_pair_wildcard_singleton(); });
	It("on_add_with_pair_singleton", [&]() { Observer_on_add_with_pair_singleton(); });
	It("add_in_yield_existing", [&]() { Observer_add_in_yield_existing(); });
	It("add_in_yield_existing_multi", [&]() { Observer_add_in_yield_existing_multi(); });
	It("name_from_root", [&]() { Observer_name_from_root(); });
	It("term_index", [&]() { Observer_term_index(); });
	It("implicit_register_in_emit_for_named_entity", [&]() { Observer_implicit_register_in_emit_for_named_entity(); });
	It("add_to_named_in_emit_for_named_entity", [&]() { Observer_add_to_named_in_emit_for_named_entity(); });
	It("implicit_register_in_emit_for_named_entity_w_defer", [&]() { Observer_implicit_register_in_emit_for_named_entity_w_defer(); });
	It("add_to_named_in_emit_for_named_entity_w_defer", [&]() { Observer_add_to_named_in_emit_for_named_entity_w_defer(); });
	It("register_twice_w_each", [&]() { Observer_register_twice_w_each(); });
	It("register_twice_w_run", [&]() { Observer_register_twice_w_run(); });
	It("register_twice_w_run_each", [&]() { Observer_register_twice_w_run_each(); });
	It("register_twice_w_each_run", [&]() { Observer_register_twice_w_each_run(); });
	It("other_table", [&]() { Observer_other_table(); });
	It("other_table_w_pair", [&]() { Observer_other_table_w_pair(); });
	It("other_table_w_pair_wildcard", [&]() { Observer_other_table_w_pair_wildcard(); });
	It("on_add_inherited", [&]() { Observer_on_add_inherited(); });
	It("on_set_inherited", [&]() { Observer_on_set_inherited(); });
	It("on_remove_inherited", [&]() { Observer_on_remove_inherited(); });
	It("on_set_after_remove_override", [&]() { Observer_on_set_after_remove_override(); });
	It("on_set_after_remove_override_create_observer_before", [&]() { Observer_on_set_after_remove_override_create_observer_before(); });
	It("on_set_w_override_after_delete", [&]() { Observer_on_set_w_override_after_delete(); });
	It("on_set_w_override_after_clear", [&]() { Observer_on_set_w_override_after_clear(); });
	It("trigger_on_set_in_on_add_implicit_registration", [&]() { Observer_trigger_on_set_in_on_add_implicit_registration(); });
	It("trigger_on_set_in_on_add_implicit_registration_namespaced", [&]() { Observer_trigger_on_set_in_on_add_implicit_registration_namespaced(); });
	It("fixed_src_w_each", [&]() { Observer_fixed_src_w_each(); });
	It("fixed_src_w_run", [&]() { Observer_fixed_src_w_run(); });
}

#endif // WITH_AUTOMATION_TESTS
