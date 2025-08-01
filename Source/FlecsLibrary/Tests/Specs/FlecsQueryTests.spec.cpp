﻿#if WITH_AUTOMATION_TESTS && defined(FLECS_TESTS)

#include "flecs.h"

#include "Misc/AutomationTest.h"
#include "Bake/FlecsTestUtils.h"
#include "Bake/FlecsTestTypes.h"

struct FlecsTestPair {
	float value;
};

struct GenericLambdaFindEntity {
	template <typename E, typename P, typename V, typename M>
	bool operator()(E&&, P&&, V&&, M&&) const {
		static_assert(flecs::is_same<E, flecs::entity>::value, "");
		static_assert(flecs::is_same<P, Position&>::value, "");
		static_assert(flecs::is_same<V, const Velocity&>::value, "");
		static_assert(flecs::is_same<M, Mass*>::value, "");
		return true;
	}
};

struct GenericLambdaFindIter {
	template <typename T, typename I, typename P, typename V, typename M>
	bool operator()(T&&, I&&, P&&, V&&, M&&) const {
		static_assert(flecs::is_same<T, flecs::iter&>::value, "");
		static_assert(flecs::is_same<I, size_t&>::value, "");
		static_assert(flecs::is_same<P, Position&>::value, "");
		static_assert(flecs::is_same<V, const Velocity&>::value, "");
		static_assert(flecs::is_same<M, Mass*>::value, "");
		return true;
	}
};

struct GenericLambdaFindComps {
	template <typename P, typename V, typename M>
	bool operator()(P&&, V&&, M&&) const {
		static_assert(flecs::is_same<P, Position&>::value, "");
		static_assert(flecs::is_same<V, const Velocity&>::value, "");
		static_assert(flecs::is_same<M, Mass*>::value, "");
		return true;
	}
};

// Generic lambdas are a C++14 feature.

struct GenericLambdaEachEntity {
	template <typename E, typename P, typename V, typename M>
	void operator()(E&&, P&&, V&&, M&&) const {
		static_assert(flecs::is_same<E, flecs::entity>::value, "");
		static_assert(flecs::is_same<P, Position&>::value, "");
		static_assert(flecs::is_same<V, const Velocity&>::value, "");
		static_assert(flecs::is_same<M, Mass*>::value, "");
	}
};

struct GenericLambdaEachIter {
	template <typename T, typename I, typename P, typename V, typename M>
	void operator()(T&&, I&&, P&&, V&&, M&&) const {
		static_assert(flecs::is_same<T, flecs::iter&>::value, "");
		static_assert(flecs::is_same<I, size_t&>::value, "");
		static_assert(flecs::is_same<P, Position&>::value, "");
		static_assert(flecs::is_same<V, const Velocity&>::value, "");
		static_assert(flecs::is_same<M, Mass*>::value, "");
	}
};

struct GenericLambdaEachComps {
	template <typename P, typename V, typename M>
	void operator()(P&&, V&&, M&&) const {
		static_assert(flecs::is_same<P, Position&>::value, "");
		static_assert(flecs::is_same<V, const Velocity&>::value, "");
		static_assert(flecs::is_same<M, Mass*>::value, "");
	}
};

#include <iostream>

struct Eats { int amount; };
struct Apples { };
struct Pears { };

struct Event {
	const char *value;
};

struct Begin { };
struct End { };

struct VelocityDerived : public Velocity {
	VelocityDerived() { }

	VelocityDerived(float _x, float _y, float _z) {
		x = _x;
		y = _y;
		z = _z;
	}

	float z;
};


using BeginEvent = flecs::pair<Begin, Event>;
using EndEvent = flecs::pair<End, Event>;

static
int compare_position(
	flecs::entity_t e1,
	const Position *p1,
	flecs::entity_t e2,
	const Position *p2)
{
	return (p1->x > p2->x) - (p1->x < p2->x);
}

static int invoked_count = 0;

BEGIN_DEFINE_SPEC(FFlecsQueryTestsSpec,
                  "FlecsLibrary.Query",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

void Query_term_each_component(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto e_1 = ecs.entity().set<Position>({1, 2});
	auto e_2 = ecs.entity().set<Position>({3, 4});
	auto e_3 = ecs.entity().set<Position>({5, 6});

	e_3.add<Tag>();

	int32_t count = 0;
	ecs.each<Position>([&](flecs::entity e, Position& p) {
		if (e == e_1) {
			test_int(p.x, 1);
			test_int(p.y, 2);
			count ++;
		}
		if (e == e_2) {
			test_int(p.x, 3);
			test_int(p.y, 4);
			count ++;
		}
		if (e == e_3) {
			test_int(p.x, 5);
			test_int(p.y, 6);
			count ++;
		}                
	});

	test_int(count, 3);
}

void Query_term_each_tag(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	struct Foo { };
	ecs.component<Foo>();

	auto e_1 = ecs.entity().add<Foo>();
	auto e_2 = ecs.entity().add<Foo>();
	auto e_3 = ecs.entity().add<Foo>();

	e_3.add<Tag>();

	int32_t count = 0;
	ecs.each<Foo>([&](flecs::entity e, Foo) {
		if (e == e_1 || e == e_2 || e == e_3) {
			count ++;
		}            
	});

	test_int(count, 3);
}


void Query_term_each_id(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto foo = ecs.entity();

	auto e_1 = ecs.entity().add(foo);
	auto e_2 = ecs.entity().add(foo);
	auto e_3 = ecs.entity().add(foo);

	e_3.add<Tag>();

	int32_t count = 0;
	ecs.each(foo, [&](flecs::entity e) {
		if (e == e_1 || e == e_2 || e == e_3) {
			count ++;
		}            
	});

	test_int(count, 3);
}


void Query_term_each_pair_type(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);
	
	struct Obj { };
	ecs.component<Obj>();

	auto e_1 = ecs.entity().add<Rel, Obj>();
	auto e_2 = ecs.entity().add<Rel, Obj>();
	auto e_3 = ecs.entity().add<Rel, Obj>();

	e_3.add<Tag>();

	int32_t count = 0;
	ecs.each<flecs::pair<Rel, Obj>>([&](flecs::entity e, flecs::pair<Rel,Obj>) {
		if (e == e_1 || e == e_2 || e == e_3) {
			count ++;
		}
	});

	test_int(count, 3);
}


void Query_term_each_pair_id(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto rel = ecs.entity();
	auto obj = ecs.entity();

	auto e_1 = ecs.entity().add(rel, obj);
	auto e_2 = ecs.entity().add(rel, obj);
	auto e_3 = ecs.entity().add(rel, obj);

	e_3.add<Tag>();

	int32_t count = 0;
	ecs.each(ecs.pair(rel, obj), [&](flecs::entity e) {
		if (e == e_1 || e == e_2 || e == e_3) {
			count ++;
		}
	});

	test_int(count, 3);
}

void Query_term_each_pair_relation_wildcard(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto rel_1 = ecs.entity();
	auto rel_2 = ecs.entity();
	auto obj = ecs.entity();

	auto e_1 = ecs.entity().add(rel_1, obj);
	auto e_2 = ecs.entity().add(rel_1, obj);
	auto e_3 = ecs.entity().add(rel_2, obj);

	e_3.add<Tag>();

	int32_t count = 0;
	ecs.each(ecs.pair(flecs::Wildcard, obj), [&](flecs::entity e) {
		if (e == e_1 || e == e_2 || e == e_3) {
			count ++;
		}
	});

	test_int(count, 3);
}

void Query_term_each_pair_object_wildcard(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto rel = ecs.entity();
	auto obj_1 = ecs.entity();
	auto obj_2 = ecs.entity();

	auto e_1 = ecs.entity().add(rel, obj_1);
	auto e_2 = ecs.entity().add(rel, obj_1);
	auto e_3 = ecs.entity().add(rel, obj_2);

	e_3.add<Tag>();

	int32_t count = 0;
	ecs.each(ecs.pair(rel, flecs::Wildcard), [&](flecs::entity e) {
		if (e == e_1 || e == e_2 || e == e_3) {
			count ++;
		}
	});

	test_int(count, 3);
}

void Query_default_ctor_no_assign(void) {
	flecs::query<> f;
    
	// Make sure code compiles & works
	test_assert(true);
}


void Query_term_get_id(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto Foo = ecs.entity();
	auto Bar = ecs.entity();

	auto q = ecs.query_builder()
		.with<Position>()
		.with<Velocity>()
		.with(Foo, Bar)
		.build();

	test_int(q.field_count(), 3);
    
	flecs::term 
	t = q.term(0);
	test_assert(t.id() == ecs.id<Position>());
	t = q.term(1);
	test_assert(t.id() == ecs.id<Velocity>());
	t = q.term(2);
	test_assert(t.id() == ecs.pair(Foo, Bar));
}

void Query_term_get_subj(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto Foo = ecs.entity();
	auto Bar = ecs.entity();
	auto Src = ecs.entity();

	auto q = ecs.query_builder()
		.with<Position>()
		.with<Velocity>().src(Src)
		.with(Foo, Bar)
		.build();

	test_int(q.field_count(), 3);
    
	flecs::term 
	t = q.term(0);
	test_assert(t.get_src() == flecs::This);
	t = q.term(1);
	test_assert(t.get_src() == Src);
	t = q.term(2);
	test_assert(t.get_src() == flecs::This);
}

void Query_term_get_pred(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto Foo = ecs.entity();
	auto Bar = ecs.entity();
	auto Src = ecs.entity();

	auto q = ecs.query_builder()
		.with<Position>()
		.with<Velocity>().src(Src)
		.with(Foo, Bar)
		.build();

	test_int(q.field_count(), 3);
    
	flecs::term 
	t = q.term(0);
	test_assert(t.get_first() == ecs.id<Position>());
	t = q.term(1);
	test_assert(t.get_first() == ecs.id<Velocity>());
	t = q.term(2);
	test_assert(t.get_first() == Foo);
}

void Query_term_get_obj(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto Foo = ecs.entity();
	auto Bar = ecs.entity();
	auto Src = ecs.entity();

	auto q = ecs.query_builder()
		.with<Position>()
		.with<Velocity>().src(Src)
		.with(Foo, Bar)
		.build();

	test_int(q.field_count(), 3);
    
	flecs::term 
	t = q.term(0);
	test_assert(t.get_second() == 0);
	t = q.term(1);
	test_assert(t.get_second() == 0);
	t = q.term(2);
	test_assert(t.get_second() == Bar);
}

void Query_get_first(void) {
	flecs::world ecs;

	struct A {};
	ecs.component<A>();

	auto e1 = ecs.entity().add<A>();
	ecs.entity().add<A>();
	ecs.entity().add<A>();

	auto q = ecs.query<A>();

	auto first = q.iter().first();
	test_assert(first != 0);
	test_assert(first == e1);
}


void Query_get_count_direct(void) {
	flecs::world ecs;

	struct A {};
	ecs.component<A>();

	ecs.entity().add<A>();
	ecs.entity().add<A>();
	ecs.entity().add<A>();

	auto q = ecs.query<A>();

	test_int(3, q.count());
}

void Query_get_is_true_direct(void) {
	flecs::world ecs;

	struct A {};
	struct B {};
	ecs.component<A>();
	ecs.component<B>();

	ecs.entity().add<A>();
	ecs.entity().add<A>();
	ecs.entity().add<A>();

	auto q_1 = ecs.query<A>();
	auto q_2 = ecs.query<B>();

	test_bool(true, q_1.is_true());
	test_bool(false, q_2.is_true());
}

void Query_get_first_direct(void) {
	flecs::world ecs;

	struct A {};
	ecs.component<A>();

	auto e1 = ecs.entity().add<A>();
	ecs.entity().add<A>();
	ecs.entity().add<A>();

	auto q = ecs.query<A>();

	auto first = q.first();
	test_assert(first != 0);
	test_assert(first == e1);
}

void Query_each_w_no_this(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto e = ecs.entity()
		.set<Position>({10, 20})
		.set<Velocity>({1, 2});

	auto f = ecs.query_builder<Position, Velocity>()
		.term_at(0).src(e)
		.term_at(1).src(e)
		.build();

	int32_t count = 0;

	f.each([&](Position& p, Velocity& v) {
		count ++;
		test_int(p.x, 10);
		test_int(p.y, 20);
		test_int(v.x, 1);
		test_int(v.y, 2);
	});

	test_int(count, 1);
}

void Query_each_w_iter_no_this(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto e = ecs.entity()
		.set<Position>({10, 20})
		.set<Velocity>({1, 2});

	auto f = ecs.query_builder<Position, Velocity>()
		.term_at(0).src(e)
		.term_at(1).src(e)
		.build();

	int32_t count = 0;

	f.each([&](flecs::iter& it, size_t index, Position& p, Velocity& v) {
		count ++;
		test_int(p.x, 10);
		test_int(p.y, 20);
		test_int(v.x, 1);
		test_int(v.y, 2);
		test_int(index, 0);
		test_int(it.count(), 0);
	});

	test_int(count, 1);
}

void Query_named_query(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto e1 = ecs.entity().add<Position>();
	auto e2 = ecs.entity().add<Position>();

	auto q = ecs.query<Position>("my_query");

	int32_t count = 0;
	q.each([&](flecs::entity e, Position&) {
		test_assert(e == e1 || e == e2);
		count ++;
	});
	test_int(count, 2);

	flecs::entity qe = q.entity();
	test_assert(qe != 0);
	test_str(qe.name(), "my_query");
}


void Query_named_scoped_query(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto e1 = ecs.entity().add<Position>();
	auto e2 = ecs.entity().add<Position>();

	auto q = ecs.query<Position>("my::query");

	int32_t count = 0;
	q.each([&](flecs::entity e, Position&) {
		test_assert(e == e1 || e == e2);
		count ++;
	});
	test_int(count, 2);

	flecs::entity qe = q.entity();
	test_assert(qe != 0);
	test_str(qe.name(), "query");
	test_str(qe.path(), "::my::query");
}


void Query_set_this_var(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	/* auto e_1 = */ ecs.entity().set<Position>({1, 2});
	auto e_2 = ecs.entity().set<Position>({3, 4});
	/* auto e_3 = */ ecs.entity().set<Position>({5, 6});

	auto q = ecs.query<Position>("my::query");

	int32_t count = 0;
	q.iter().set_var(0, e_2).each([&](flecs::entity e, Position&) {
		test_assert(e == e_2);
		count ++;
	});
	test_int(count, 1);
}

void Query_inspect_terms_w_expr(void) {
	flecs::world ecs;

	flecs::query<> f = ecs.query_builder()
		.expr("(ChildOf,#0)")
		.build();

	int32_t count = 0;
	f.each_term([&](flecs::term &term) {
		test_assert(term.id().is_pair());
		count ++;
	});

	test_int(count, 1);
}

void Query_find(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	/* auto e1 = */ ecs.entity().set<Position>({10, 20});
	auto e2 = ecs.entity().set<Position>({20, 30});

	auto q = ecs.query<Position>();

	auto r = q.find([](Position& p) {
		return p.x == 20;
	});

	test_assert(r == e2);
}

void Query_find_not_found(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	/* auto e1 = */ ecs.entity().set<Position>({10, 20});
	/* auto e2 = */ ecs.entity().set<Position>({20, 30});

	auto q = ecs.query<Position>();

	auto r = q.find([](Position& p) {
		return p.x == 30;
	});

	test_assert(!r);
}

void Query_find_w_entity(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	/* auto e1 = */ ecs.entity().set<Position>({10, 20}).set<Velocity>({20, 30});
	auto e2 = ecs.entity().set<Position>({20, 30}).set<Velocity>({20, 30});

	auto q = ecs.query<Position>();

	auto r = q.find([](flecs::entity e, Position& p) {
		return p.x == e.try_get<Velocity>()->x &&
			   p.y == e.try_get<Velocity>()->y;
	});

	test_assert(r == e2);
}

void Query_find_w_match_empty_tables(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto e1 = ecs.entity().set<Position>({10, 20}).add<Velocity>();
	e1.destruct(); // creates empty table
	auto e2 = ecs.entity().set<Position>({20, 30});

	auto q = ecs.query_builder<Position>()
		.query_flags(EcsQueryMatchEmptyTables)
		.build();

	auto r = q.find([](Position& p) {
		return p.x == 20;
	});

	test_assert(r == e2);
}

void Query_find_w_entity_w_match_empty_tables(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto e1 = ecs.entity().set<Position>({10, 20}).add<Velocity>();
	e1.destruct(); // creates empty table
	auto e2 = ecs.entity().set<Position>({20, 30});

	auto q = ecs.query_builder<Position>()
		.query_flags(EcsQueryMatchEmptyTables)
		.build();

	auto r = q.find([](flecs::entity e, Position& p) {
		return p.x == 20;
	});

	test_assert(r == e2);
}

void Query_find_generic(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	auto q = world.query<Position, const Velocity, Mass*>();

	q.find(GenericLambdaFindEntity{});
	q.find(GenericLambdaFindIter{});
	q.find(GenericLambdaFindComps{});
}

void Query_optional_pair_term(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	ecs.entity()
		.add<Tag0>()
		.emplace<Position, Tag>(1.0f, 2.0f);
	ecs.entity()
		.add<Tag0>();

	int32_t with_pair = 0, without_pair = 0;

	auto f = ecs.query_builder<flecs::pair<Position, Tag>*>()
		.with<Tag0>()
		.build();
        
	f.each([&](flecs::entity e, Position* p) {
		if (p) {
			with_pair++;
			test_flt(1.0f, p->x);
			test_flt(2.0f, p->y);
		} else {
			without_pair++;
		}
	});

	ecs.progress(1.0);

	test_int(1, with_pair);
	test_int(1, without_pair);
}


void Query_run(void) {
    flecs::world world;
	RegisterTestTypeComponents(world);

    world.component<Position>();
    world.component<Velocity>();

    auto entity = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    auto q = world.query<Position, Velocity>();

    q.run([](flecs::iter& it) {
        while (it.next()) {
            auto p = it.field<Position>(0);
            auto v = it.field<Velocity>(1);

            for (auto i : it) {
                p[i].x += v[i].x;
                p[i].y += v[i].y;
            }
        }
    });

    const Position *p = entity.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);
}


void Query_run_const(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	world.component<Position>();
	world.component<Velocity>();

	auto entity = world.entity()
		.set<Position>({10, 20})
		.set<Velocity>({1, 2});

	auto q = world.query<Position, const Velocity>();

	q.run([](flecs::iter& it) {
		while (it.next()) {
			auto p = it.field<Position>(0);
			auto v = it.field<const Velocity>(1);

			for (auto i : it) {
				p[i].x += v[i].x;
				p[i].y += v[i].y;
			}
		}
	});

	const Position *p = entity.try_get<Position>();
	test_int(p->x, 11);
	test_int(p->y, 22);
}

void Query_run_shared(void) {
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

	auto q = world.query_builder<Position>()
		.expr("Velocity(self|up IsA)")
		.build();

	q.run([](flecs::iter&it) {
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

	const Position *p = e1.try_get<Position>();
	test_int(p->x, 11);
	test_int(p->y, 22);

	p = e2.try_get<Position>();
	test_int(p->x, 13);
	test_int(p->y, 24);  
}

void Query_run_optional(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	world.component<Position>();
	world.component<Velocity>();
	flecs::component<Mass>(world, "Mass");

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

	auto q = world.query<Position, Velocity*, Mass*>();

	q.run([](flecs::iter& it) {
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

void Query_run_sparse(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	world.component<Position>().add(flecs::Sparse);
	world.component<Velocity>();

	auto entity = world.entity()
		.set<Position>({10, 20})
		.set<Velocity>({1, 2});

	auto q = world.query<Position, Velocity>();

	q.run([](flecs::iter& it) {
		while (it.next()) {
			auto v = it.field<Velocity>(1);

			for (auto i : it) {
				auto& p = it.field_at<Position>(0, i);
				p.x += v[i].x;
				p.y += v[i].y;
			}
		}
	});

	const Position *p = entity.try_get<Position>();
	test_int(p->x, 11);
	test_int(p->y, 22);
}

void Query_run_sparse_w_with(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	world.component<Position>().add(flecs::Sparse);
	world.component<Velocity>();

	auto entity = world.entity()
		.set<Position>({10, 20})
		.set<Velocity>({1, 2});

	auto q = world.query_builder()
		.with<Position>()
		.with<Velocity>()
		.build();

	q.run([](flecs::iter& it) {
		while (it.next()) {
			auto v = it.field<Velocity>(1);

			for (auto i : it) {
				auto& p = it.field_at<Position>(0, i);
				p.x += v[i].x;
				p.y += v[i].y;
			}
		}
	});

	const Position *p = entity.try_get<Position>();
	test_int(p->x, 11);
	test_int(p->y, 22);
}

void Query_run_dont_fragment(void) {
    flecs::world world;

    world.component<Position>().add(flecs::DontFragment);
    world.component<Velocity>();

    auto entity = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    auto q = world.query<Position, Velocity>();

    q.run([](flecs::iter& it) {
        while (it.next()) {
            auto v = it.field<Velocity>(1);

            for (auto i : it) {
                auto& p = it.field_at<Position>(0, i);
                p.x += v[i].x;
                p.y += v[i].y;
            }
        }
    });

    const Position *p = entity.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);
}

void Query_run_dont_fragment_w_with(void) {
    flecs::world world;

    world.component<Position>().add(flecs::DontFragment);
    world.component<Velocity>();

    auto entity = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    auto q = world.query_builder()
        .with<Position>()
        .with<Velocity>()
        .build();

    q.run([](flecs::iter& it) {
        while (it.next()) {
            auto v = it.field<Velocity>(1);

            for (auto i : it) {
                auto& p = it.field_at<Position>(0, i);
                p.x += v[i].x;
                p.y += v[i].y;
            }
        }
    });

    const Position *p = entity.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);
}

void Query_run_dont_fragment_add(void) {
    flecs::world world;

    world.component<Position>();
    world.component<Velocity>().add(flecs::DontFragment);

    auto entity = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    auto q = world.query<Position>();

    q.run([](flecs::iter& it) {
        while (it.next()) {
            for (auto i : it) {
                flecs::entity e = it.entity(i);
                e.add<Velocity>();
                test_assert(e.has<Velocity>());

                auto& p = it.field_at<Position>(0, i);
                p.x += 1;
                p.y += 2;
            }
        }
    });

    const Position *p = entity.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);

    const Velocity *v = entity.try_get<Velocity>();
    test_assert(v != nullptr);
}

void Query_run_dont_fragment_add_remove(void) {
    flecs::world world;

    world.component<Position>();
    world.component<Velocity>().add(flecs::DontFragment);

    auto entity = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    auto q = world.query<Position>();

    q.run([](flecs::iter& it) {
        while (it.next()) {
            for (auto i : it) {
                flecs::entity e = it.entity(i);
                e.add<Velocity>();
                test_assert(e.has<Velocity>());

                e.remove<Velocity>();
                test_assert(!e.has<Velocity>());

                auto& p = it.field_at<Position>(0, i);
                p.x += 1;
                p.y += 2;
            }
        }
    });

    const Position *p = entity.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);

    const Velocity *v = entity.try_get<Velocity>();
    test_assert(v == nullptr);
}

void Query_run_dont_fragment_set(void) {
    flecs::world world;

    world.component<Position>();
    world.component<Velocity>().add(flecs::DontFragment);

    auto entity = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    auto q = world.query<Position>();

    q.run([](flecs::iter& it) {
        while (it.next()) {
            for (auto i : it) {
                flecs::entity e = it.entity(i);
                e.set<Velocity>({1, 2});
                test_assert(e.has<Velocity>());
                {
                    const Velocity *v = e.try_get<Velocity>();
                    test_int(v->x, 1);
                    test_int(v->y, 2);
                }

                auto& p = it.field_at<Position>(0, i);
                p.x += 1;
                p.y += 2;
            }
        }
    });

    const Position *p = entity.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);

    const Velocity *v = entity.try_get<Velocity>();
    test_int(v->x, 1);
    test_int(v->y, 2);
}

void Query_each(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	world.component<Position>();
	world.component<Velocity>();

	auto entity = world.entity()
		.set<Position>({10, 20})
		.set<Velocity>({1, 2});

	auto q = world.query<Position, Velocity>();

	q.each([](flecs::entity e, Position& p, Velocity& v) {
		p.x += v.x;
		p.y += v.y;
	});

	const Position *p = entity.try_get<Position>();
	test_int(p->x, 11);
	test_int(p->y, 22);
}

void Query_each_const(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	world.component<Position>();
	world.component<Velocity>();

	auto entity = world.entity()
		.set<Position>({10, 20})
		.set<Velocity>({1, 2});

	auto q = world.query<Position, const Velocity>();

	q.each([](flecs::entity e, Position& p, const Velocity& v) {
		p.x += v.x;
		p.y += v.y;
	});

	const Position *p = entity.try_get<Position>();
	test_int(p->x, 11);
	test_int(p->y, 22);
}

void Query_each_shared(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	world.component<Position>();
	world.component<Velocity>();

	auto base = world.entity()
		.set<Velocity>({1, 2});

	auto e1 = world.entity()
		.set<Position>({10, 20})
		.add(flecs::IsA, base);

	auto e2 = world.entity()
		.set<Position>({20, 30})
		.add(flecs::IsA, base);

	auto e3 = world.entity()
		.set<Position>({10, 20})
		.set<Velocity>({3, 4});

	auto q = world.query<Position, const Velocity>();

	q.each([](flecs::entity e, Position& p, const Velocity& v) {
		p.x += v.x;
		p.y += v.y;
	});

	const Position *p = e1.try_get<Position>();
	test_int(p->x, 11);
	test_int(p->y, 22);

	p = e2.try_get<Position>();
	test_int(p->x, 21);
	test_int(p->y, 32);

	p = e3.try_get<Position>();
	test_int(p->x, 13);
	test_int(p->y, 24); 
}

void Query_each_optional(void) {
	flecs::world world;

	world.component<Position>();
	world.component<Velocity>();
	flecs::component<Mass>(world, "Mass");

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

	auto q = world.query<Position, Velocity*, Mass*>();

	q.each([](flecs::entity e, Position& p, Velocity* v, Mass *m) {
		if (v && m) {
			p.x += v->x * m->value;
			p.y += v->y * m->value;
		} else {
			p.x ++;
			p.y ++;
		}
	});

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

void Query_each_sparse(void) {
	flecs::world world;

	world.component<Position>().add(flecs::Sparse);
	world.component<Velocity>();

	auto entity = world.entity()
		.set<Position>({10, 20})
		.set<Velocity>({1, 2});

	auto q = world.query<Position, Velocity>();

	q.each([](Position& p, Velocity& v) {
		p.x += v.x;
		p.y += v.y;
	});

	const Position *p = entity.try_get<Position>();
	test_int(p->x, 11);
	test_int(p->y, 22);
}

void Query_each_sparse_w_with(void) {
	flecs::world world;

	world.component<Position>().add(flecs::Sparse);
	world.component<Velocity>();

	auto entity = world.entity()
		.set<Position>({10, 20})
		.set<Velocity>({1, 2});

	auto q = world.query_builder()
		.with<Position>()
		.with<Velocity>()
		.build();

	q.each([](flecs::iter& it, size_t row) {
		Position& p = it.field_at<Position>(0, row);
		Velocity& v = it.field_at<Velocity>(1, row);
		p.x += v.x;
		p.y += v.y;
	});

	const Position *p = entity.try_get<Position>();
	test_int(p->x, 11);
	test_int(p->y, 22);
}

void Query_each_sparse_many(void) {
	flecs::world world;

	world.component<Position>().add(flecs::Sparse);
	world.component<Velocity>();
    
	std::vector<flecs::entity> entities;

	for (int i = 0; i < 2000; i ++) {
		entities.push_back(world.entity()
			.set<Position>({
				static_cast<float>(10 + i), 
				static_cast<float>(20 + i)
			})
			.set<Velocity>({
				static_cast<float>(i), 
				static_cast<float>(i)
			}));
	}

	auto q = world.query<Position, Velocity>();

	q.each([](Position& p, Velocity& v) {
		p.x += v.x;
		p.y += v.y;
	});

	for (int i = 0; i < 2000; i ++) {
		flecs::entity e = entities[i];
		const Position *p = e.try_get<Position>();
		test_int(p->x, 10 + i * 2);
		test_int(p->y, 20 + i * 2);
		const Velocity *v = e.try_get<Velocity>();
		test_int(v->x, i);
		test_int(v->y, i);
	}
}

void Query_each_dont_fragment(void) {
    flecs::world world;

    world.component<Position>().add(flecs::DontFragment);
    world.component<Velocity>();

    auto entity = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    auto q = world.query<Position, Velocity>();

    q.each([](Position& p, Velocity& v) {
        p.x += v.x;
        p.y += v.y;
    });

    const Position *p = entity.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);
}

void Query_each_dont_fragment_w_with(void) {
    flecs::world world;

    world.component<Position>().add(flecs::DontFragment);
    world.component<Velocity>();

    auto entity = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    auto q = world.query_builder()
        .with<Position>()
        .with<Velocity>()
        .build();

    q.each([](flecs::iter& it, size_t row) {
        Position& p = it.field_at<Position>(0, row);
        Velocity& v = it.field_at<Velocity>(1, row);
        p.x += v.x;
        p.y += v.y;
    });

    const Position *p = entity.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);
}

void Query_each_dont_fragment_many(void) {
    flecs::world world;

    world.component<Position>().add(flecs::DontFragment);
    world.component<Velocity>();
    
    std::vector<flecs::entity> entities;

    for (int i = 0; i < 2000; i ++) {
        entities.push_back(world.entity()
            .set<Position>({
                static_cast<float>(10 + i), 
                static_cast<float>(20 + i)
            })
            .set<Velocity>({
                static_cast<float>(i), 
                static_cast<float>(i)
            }));
    }

    auto q = world.query<Position, Velocity>();

    q.each([](Position& p, Velocity& v) {
        p.x += v.x;
        p.y += v.y;
    });

    for (int i = 0; i < 2000; i ++) {
        flecs::entity e = entities[i];
        const Position *p = e.try_get<Position>();
        test_int(p->x, 10 + i * 2);
        test_int(p->y, 20 + i * 2);
        const Velocity *v = e.try_get<Velocity>();
        test_int(v->x, i);
        test_int(v->y, i);
    }
}

void Query_each_dont_fragment_add(void) {
    flecs::world world;

    world.component<Position>();
    world.component<Velocity>().add(flecs::DontFragment);

    auto entity = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    auto q = world.query<Position>();

    q.each([](flecs::entity e, Position& p) {
        e.add<Velocity>();
        test_assert(e.has<Velocity>());

        p.x += 1;
        p.y += 2;
    });

    const Position *p = entity.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);

    test_assert(entity.has<Velocity>());
}

void Query_each_dont_fragment_add_remove(void) {
    flecs::world world;

    world.component<Position>();
    world.component<Velocity>().add(flecs::DontFragment);

    auto entity = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    auto q = world.query<Position>();

    q.each([](flecs::entity e, Position& p) {
        e.add<Velocity>();
        test_assert(e.has<Velocity>());

        e.remove<Velocity>();
        test_assert(!e.has<Velocity>());

        p.x += 1;
        p.y += 2;
    });

    const Position *p = entity.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);

    test_assert(!entity.has<Velocity>());
}

void Query_each_dont_fragment_set(void) {
    flecs::world world;

    world.component<Position>();
    world.component<Velocity>().add(flecs::DontFragment);

    auto entity = world.entity()
        .set<Position>({10, 20})
        .set<Velocity>({1, 2});

    auto q = world.query<Position>();

    q.each([](flecs::entity e, Position& p) {
        e.set<Velocity>({1, 2});
        test_assert(e.has<Velocity>());
        {
            const Velocity *v = e.try_get<Velocity>();
            test_assert(v != nullptr);
            test_int(v->x, 1);
            test_int(v->y, 2);
        }

        p.x += 1;
        p.y += 2;
    });

    const Position *p = entity.try_get<Position>();
    test_int(p->x, 11);
    test_int(p->y, 22);

    const Velocity *v = entity.try_get<Velocity>();
    test_int(v->x, 1);
    test_int(v->y, 2);
}

void Query_each_generic(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	auto q = world.query<Position, const Velocity, Mass*>();

	q.each(GenericLambdaEachEntity{});
	q.each(GenericLambdaEachIter{});
	q.each(GenericLambdaEachComps{});
}

void Query_signature(void) {
	flecs::world world;

	world.component<Position>();
	world.component<Velocity>();

	auto entity = world.entity()
		.set<Position>({10, 20})
		.set<Velocity>({1, 2});

	auto q = world.query_builder<>().expr("Position, Velocity").build();

	q.run([](flecs::iter& it) {
		while (it.next()) {
			auto p = it.field<Position>(0);
			auto v = it.field<Velocity>(1);

			for (auto i : it) {
				p[i].x += v[i].x;
				p[i].y += v[i].y;
			}
		}
	});

	const Position *p = entity.try_get<Position>();
	test_int(p->x, 11);
	test_int(p->y, 22);
}

void Query_signature_const(void) {
	flecs::world world;

	world.component<Position>();
	world.component<Velocity>();

	auto entity = world.entity()
		.set<Position>({10, 20})
		.set<Velocity>({1, 2});

	auto q = world.query_builder<>().expr("Position, [in] Velocity").build();

	q.run([](flecs::iter& it) {        
		while (it.next()) {
			auto p = it.field<Position>(0);
			auto v = it.field<const Velocity>(1);

			for (auto i : it) {
				p[i].x += v[i].x;
				p[i].y += v[i].y;
			}
		}
	});

	const Position *p = entity.try_get<Position>();
	test_int(p->x, 11);
	test_int(p->y, 22);
}

void Query_signature_shared(void) {
	flecs::world world;

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

	auto q = world.query_builder<>()
		.expr("Position, [in] Velocity(self|up IsA)")
		.build();
    
	q.run([](flecs::iter&it) {
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

	const Position *p = e1.try_get<Position>();
	test_int(p->x, 11);
	test_int(p->y, 22);

	p = e2.try_get<Position>();
	test_int(p->x, 13);
	test_int(p->y, 24); 
}

void Query_signature_optional(void) {
	flecs::world world;

	world.component<Position>();
	world.component<Velocity>();
	flecs::component<Mass>(world, "Mass");

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

	auto q = world.query_builder<>().expr("Position, ?Velocity, ?Mass").build();

	q.run([](flecs::iter& it) {
		while (it.next()) {
			auto p = it.field<Position>(0);
			auto v = it.field<const Velocity>(1);
			auto m = it.field<const Mass>(2);

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

void Query_query_single_pair(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);
	world.component<FlecsTestPair>();

	world.entity().add<FlecsTestPair, Position>();
	auto e2 = world.entity().add<FlecsTestPair, Velocity>();
    
	auto q = world.query_builder<>()
		.expr("(FlecsTestPair, Velocity)")
		.build();

	int32_t table_count = 0;
	int32_t entity_count = 0;

	q.run([&](flecs::iter it) {
		while (it.next()) {
			table_count ++;
			for (auto i : it) {
				test_assert(it.entity(i) == e2);
				entity_count ++;
			}
		}
	});

	test_int(table_count, 1);
	test_int(entity_count, 1);
}

void Query_tag_w_each(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	auto q = world.query<Tag>();

	auto e = world.entity()
		.add<Tag>();

	q.each([&](flecs::entity qe, Tag) {
		test_assert(qe == e);
	});
}

void Query_shared_tag_w_each(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	auto q = world.query<Tag>();

	auto base = world.prefab()
		.add<Tag>();

	auto e = world.entity()
		.add(flecs::IsA, base);

	q.each([&](flecs::entity qe, Tag) {
		test_assert(qe == e);
	});
}

void Query_sort_by(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	world.entity().set<Position>({1, 0});
	world.entity().set<Position>({6, 0});
	world.entity().set<Position>({2, 0});
	world.entity().set<Position>({5, 0});
	world.entity().set<Position>({4, 0});

	auto q = world.query_builder<Position>()
		.order_by(compare_position)
		.build();

	q.run([](flecs::iter it) {
		while (it.next()) {
			auto p = it.field<Position>(0);
			test_int(it.count(), 5);
			test_int(p[0].x, 1);
			test_int(p[1].x, 2);
			test_int(p[2].x, 4);
			test_int(p[3].x, 5);
			test_int(p[4].x, 6);
		}
	});
}

void Query_changed(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	auto e = world.entity().set<Position>({1, 0});

	auto q = world.query_builder<const Position>()
		.detect_changes()
		.cached()
		.build();

	auto q_w = world.query<Position>();

	test_bool(q.changed(), true);

	q.each([](const Position& p) { });
	test_bool(q.changed(), false);
    
	e.set<Position>({2, 0});
	test_bool(q.changed(), true);

	q.each([](const Position& p) { });
	test_bool(q.changed(), false); // Reset state

	q_w.each([](Position& p) { }); // Query has out term
	test_bool(q.changed(), true);
}


void Query_default_ctor(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	flecs::query<Position> q_var;

	int count = 0;
	auto q = world.query<Position>();

	world.entity().set<Position>({10, 20});

	q_var = q;
    
	q_var.each([&](flecs::entity e, Position& p) {
		test_int(p.x, 10);
		test_int(p.y, 20);
		count ++;
	});

	test_int(count, 1);
}

void Query_expr_w_template(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	auto comp = world.component<Template<int>>();
	test_str(comp.name(), "Template<int>");

	int count = 0;
	auto q = world.query_builder<Position>().expr("Template<int>").build();

	world.entity()
		.set<Position>({10, 20})
		.set<Template<int>>({30, 40});
    
	q.each([&](flecs::entity e, Position& p) {
		test_int(p.x, 10);
		test_int(p.y, 20);

		const Template<int> *t = e.try_get<Template<int>>();
		test_int(t->x, 30);
		test_int(t->y, 40);        

		count ++;
	});

	test_int(count, 1);
}

void Query_query_type_w_template(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	auto comp = world.component<Template<int>>();
	test_str(comp.name(), "Template<int>");

	int count = 0;
	auto q = world.query<Position, Template<int>>();

	world.entity()
		.set<Position>({10, 20})
		.set<Template<int>>({30, 40});
    
	q.each([&](flecs::entity e, Position& p, Template<int>& t) {
		test_int(p.x, 10);
		test_int(p.y, 20);

		test_int(t.x, 30);
		test_int(t.y, 40);        

		count ++;
	});

	test_int(count, 1);
}

void Query_compare_term_id(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	int count = 0;
	auto e = world.entity().add<Tag>();

	auto q = world.query_builder<>()
		.with<Tag>()
		.build();
    
	q.run([&](flecs::iter& it) {
		while (it.next()) {
			test_assert(it.id(0) == it.world().id<Tag>());
			test_assert(it.entity(0) == e);
		}
		count ++;
	});

	test_int(count, 1);
}

void Query_inspect_terms(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	auto p = world.entity();

	auto q = world.query_builder<Position>()
		.with<Velocity>()
		.with(flecs::ChildOf, p)
		.build();

	test_int(3, q.field_count());

	auto t = q.term(0);
	test_int(t.id(), world.id<Position>());
	test_int(t.oper(), flecs::And);
	test_int(t.inout(), flecs::InOutDefault);

	t = q.term(1);
	test_int(t.id(), world.id<Velocity>());
	test_int(t.oper(), flecs::And);
	test_int(t.inout(), flecs::InOutDefault);

	t = q.term(2);
	test_int(t.id(), world.pair(flecs::ChildOf, p));
	test_int(t.oper(), flecs::And);
	test_int(t.inout(), flecs::InOutDefault);
	test_assert(t.id().second() == p);
}

void Query_inspect_terms_w_each(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	auto p = world.entity();

	auto q = world.query_builder<Position>()
		.with<Velocity>()
		.with(flecs::ChildOf, p)
		.build();

	int32_t count =  0;
	q.each_term([&](flecs::term& t) {
		if (count == 0) {
			test_int(t.id(), world.id<Position>());
			test_int(t.inout(), flecs::InOutDefault);
		} else if (count == 1) {
			test_int(t.id(), world.id<Velocity>());
			test_int(t.inout(), flecs::InOutDefault);
		} else if (count == 2) {
			test_int(t.id(), world.pair(flecs::ChildOf, p));
			test_assert(t.id().second() == p);
			test_int(t.inout(), flecs::InOutDefault);
		} else {
			test_assert(false);
		}

		test_int(t.oper(), flecs::And);

		count ++;
	});

	test_int(count, 3);
}

void Query_comp_to_str(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto q = ecs.query_builder<Position>()
		.with<Velocity>()
		.build();
	test_str(q.str(), "Position($this), Velocity($this)");
}

void Query_pair_to_str(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);
	ecs.component<Apples>();
	ecs.component<Eats>();

	auto q = ecs.query_builder<Position>()
		.with<Velocity>()
		.with<Eats, Apples>()
		.build();
	test_str(q.str(), "Position($this), Velocity($this), Eats($this,Apples)");
}

void Query_oper_not_to_str(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto q = ecs.query_builder<Position>()
		.with<Velocity>().oper(flecs::Not)
		.build();
	test_str(q.str(), "Position($this), !Velocity($this)");
}

void Query_oper_optional_to_str(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto q = ecs.query_builder<Position>()
		.with<Velocity>().oper(flecs::Optional)
		.build();
	test_str(q.str(), "Position($this), ?Velocity($this)");
}

void Query_oper_or_to_str(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto q = ecs.query_builder<>()
		.with<Position>().oper(flecs::Or)
		.with<Velocity>()
		.build();
	test_str(q.str(), "Position($this) || Velocity($this)");
}

using EatsApples = flecs::pair<Eats, Apples>;
using EatsPears = flecs::pair<Eats, Pears>;

void Query_each_pair_type(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);
	ecs.component<Eats>();
	ecs.component<Apples>();
	ecs.component<Pears>();

	auto e1 = ecs.entity()
		.set<EatsApples>({10});

	ecs.entity()
		.set<EatsPears>({20});

	auto q = ecs.query<EatsApples>();

	int count = 0;
	q.each([&](flecs::entity e, EatsApples&& a) {
		test_int(a->amount, 10);
		test_assert(e == e1);
		a->amount ++;
		count ++;
	});

	test_int(count, 1);

	auto v = e1.try_get<EatsApples>();
	test_assert(v != NULL);
	test_int(v->amount, 11);
}

void Query_iter_pair_type(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);
	ecs.component<Eats>();
	ecs.component<Apples>();
	ecs.component<Pears>();

	auto e1 = ecs.entity()
		.set<EatsApples>({10});

	ecs.entity()
		.set<EatsPears>({20});

	auto q = ecs.query<EatsApples>();

	int count = 0;
	q.run([&](flecs::iter& it) {
		while (it.next()) {
			auto a = it.field<Eats>(0);
			test_int(it.count(), 1);

			test_int(a->amount, 10);
			test_assert(it.entity(0) == e1);

			a->amount ++;
			count ++;
		}
	});

	test_int(count, 1);

	auto v = e1.try_get<EatsApples>();
	test_assert(v != NULL);
	test_int(v->amount, 11);
}

void Query_term_pair_type(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);
	ecs.component<Eats>();
	ecs.component<Apples>();
	ecs.component<Pears>();

	auto e1 = ecs.entity()
		.set<EatsApples>({10});

	ecs.entity()
		.set<EatsPears>({20});

	auto q = ecs.query_builder<>()
		.with<EatsApples>().inout()
		.build();

	int count = 0;
	q.run([&](flecs::iter& it) {
		while (it.next()) {
			test_int(it.count(), 1);

			auto a = it.field<EatsApples>(0);

			test_int(a->amount, 10);
			test_assert(it.entity(0) == e1);

			a->amount ++;
			count ++;
		}
	});

	test_int(count, 1);

	auto v = e1.try_get<EatsApples>();
	test_assert(v != NULL);
	test_int(v->amount, 11);
}

void Query_each_no_entity_1_comp(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto e = ecs.entity()
		.set(Position{1, 2});

	auto q = ecs.query<Position>();

	int32_t count = 0;
	q.each([&](Position& p) {
		test_int(p.x, 1);
		test_int(p.y, 2);
		p.x += 1;
		p.y += 2;
		count ++;
	});

	test_int(count, 1);
    
	auto pos = e.try_get<Position>();
	test_int(pos->x, 2);
	test_int(pos->y, 4);
}

void Query_each_no_entity_2_comps(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto e = ecs.entity()
		.set(Position{1, 2})
		.set(Velocity{10, 20});

	auto q = ecs.query<Position, Velocity>();

	int32_t count = 0;
	q.each([&](Position& p, Velocity& v) {
		test_int(p.x, 1);
		test_int(p.y, 2);
		test_int(v.x, 10);
		test_int(v.y, 20);

		p.x += 1;
		p.y += 2;
		v.x += 1;
		v.y += 2;
		count ++;
	});

	test_int(count, 1);

	test_bool(e.get([](const Position& p, const Velocity& v) {
		test_int(p.x, 2);
		test_int(p.y, 4);

		test_int(v.x, 11);
		test_int(v.y, 22);
	}), true);

	test_int(count, 1);
}

void Query_iter_no_comps_1_comp(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	ecs.entity().add<Position>();
	ecs.entity().add<Position>();
	ecs.entity().add<Position>().add<Velocity>();
	ecs.entity().add<Velocity>();

	auto q = ecs.query<Position>();

	int32_t count = 0;
	q.run([&](flecs::iter& it) {
		while (it.next()) {
			count += it.count();
		}
	});

	test_int(count, 3);
}

void Query_iter_no_comps_2_comps(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	ecs.entity().add<Velocity>();
	ecs.entity().add<Position>();
	ecs.entity().add<Position>().add<Velocity>();
	ecs.entity().add<Position>().add<Velocity>();

	auto q = ecs.query<Position, Velocity>();

	int32_t count = 0;
	q.run([&](flecs::iter& it) {
		while (it.next()) {
			count += it.count();
		}
	});

	test_int(count, 2);
}


void Query_iter_no_comps_no_comps(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	ecs.entity().add<Velocity>();
	ecs.entity().add<Position>();
	ecs.entity().add<Position>().add<Velocity>();
	ecs.entity().add<Position>().add<Velocity>();

	auto q = ecs.query_builder<>()
		.with<Position>()
		.build();

	int32_t count = 0;
	q.run([&](flecs::iter& it) {
		while (it.next()) {
			count += it.count();
		}
	});

	test_int(count, 3);
}

void Query_each_pair_object(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);
	
	ecs.component<Begin>();
	ecs.component<End>();
	ecs.component<Event>();

	auto e1 = ecs.entity()
		.set_second<Begin, Event>({"Big Bang"})
		.set<EndEvent>({"Heat Death"});

	auto q = ecs.query<BeginEvent, EndEvent>();

	int32_t count = 0;
	q.each([&](flecs::entity e, BeginEvent b_e, EndEvent e_e) {
		test_assert(e == e1);
		test_str(b_e->value, "Big Bang");
		test_str(e_e->value, "Heat Death");
		count ++;
	});

	test_int(count, 1);
}


void Query_iter_pair_object(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);
	ecs.component<Begin>();
	ecs.component<End>();
	ecs.component<Event>();

	auto e1 = ecs.entity()
		.set_second<Begin, Event>({"Big Bang"})
		.set<EndEvent>({"Heat Death"});

	auto q = ecs.query<BeginEvent, EndEvent>();

	int32_t count = 0;
	q.run([&](flecs::iter it) {
		while (it.next()) {
			auto b_e = it.field<BeginEvent>(0);
			auto e_e = it.field<EndEvent>(1);

			for (auto i : it) {
				test_assert(it.entity(i) == e1);
				test_str(b_e[i].value, "Big Bang");
				test_str(e_e[i].value, "Heat Death");
				count ++;
			}
		}
	});

	test_int(count, 1);
}


void Query_iter_query_in_system(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	ecs.entity().add<Position>().add<Velocity>();

	auto q = ecs.query<Velocity>();

	int32_t count = 0;
	ecs.system<Position>()
		.each([&](flecs::entity e1, Position&) {
			q.each([&](flecs::entity e2, Velocity&) {
				count ++;
			});
		});

	ecs.progress();
 
	test_int(count, 1);
}

void Query_iter_type(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	ecs.entity().add<Position>();
	ecs.entity().add<Position>().add<Velocity>();

	auto q = ecs.query<Position>();

	q.run([&](flecs::iter it) {
		while (it.next()) {
			test_assert(it.type().count() >= 1);
			test_assert(it.table().has<Position>());
		}
	});
}

void Query_instanced_query_w_singleton_each(void) {
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

	auto q = ecs.query_builder<Self, Position, const Velocity>()
		.term_at(2)
		.build();

	int32_t count = 0;
	q.each([&](flecs::entity e, Self& s, Position&p, const Velocity& v) {
		test_assert(e == s.value);
		p.x += v.x;
		p.y += v.y;
		count ++;
	});

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

void Query_instanced_query_w_base_each(void) {
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

	auto q = ecs.query_builder<Self, Position, const Velocity>()
		.build();

	int32_t count = 0;
	q.each([&](flecs::entity e, Self& s, Position&p, const Velocity& v) {
		test_assert(e == s.value);
		p.x += v.x;
		p.y += v.y;
		count ++;
	});

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

void Query_instanced_query_w_singleton_iter(void) {
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

	auto q = ecs.query_builder<Self, Position, const Velocity>()
		.term_at(2)
		.build();

	int32_t count = 0;

	q.run([&](flecs::iter it) {
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

void Query_instanced_query_w_base_iter(void) {
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

    auto q = ecs.query_builder<Self, Position, const Velocity>()
        .build();

    int32_t count = 0;
    q.run([&](flecs::iter it) {
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

void Query_query_each_from_component(void) {
	flecs::world w;
	RegisterTestTypeComponents(w);

	w.entity().set<Position>({}).set<Velocity>({});
	w.entity().set<Position>({}).set<Velocity>({});

	struct QueryComponent {
		flecs::query<Position, Velocity> q;
	};
	w.component<QueryComponent>();

	auto q = w.query<Position, Velocity>();
	auto e = w.entity().set<QueryComponent>({ q });

	const QueryComponent *qc = e.try_get<QueryComponent>();
	test_assert(qc != nullptr);

	int count = 0;
	qc->q.each([&](Position&, Velocity&) { // Ensure we can iterate const query
		count ++;
	});
	test_int(count, 2);
}

void Query_query_iter_from_component(void) {
	flecs::world w;
	RegisterTestTypeComponents(w);

	w.entity().set<Position>({}).set<Velocity>({});
	w.entity().set<Position>({}).set<Velocity>({});

	struct QueryComponent {
		flecs::query<Position, Velocity> q;
	};
	w.component<QueryComponent>();

	auto q = w.query<Position, Velocity>();
	auto e = w.entity().set<QueryComponent>({ q });

	const QueryComponent *qc = e.try_get<QueryComponent>();
	test_assert(qc != nullptr);

	int count = 0;
	qc->q.run([&](flecs::iter& it) { // Ensure we can iterate const query
		while (it.next()) {
			count += it.count();
		}
	});
	test_int(count, 2);
}

static void EachFunc(flecs::entity e, Position& p) {
	invoked_count ++;
	p.x ++;
	p.y ++;
}

static void RunFunc(flecs::iter& it) {
	test_bool(true, it.next());
	test_int(it.count(), 1);
	auto p = it.field<Position>(0);
	invoked_count ++;
	p->x ++;
	p->y ++;
	test_bool(false, it.next());
}

void Query_query_each_w_func_ptr(void) {
	flecs::world w;
	RegisterTestTypeComponents(w);
	invoked_count = 0;

	auto e = w.entity().set<Position>({10, 20});

	auto q = w.query<Position>();

	q.each(EachFunc);

	test_int(invoked_count, 1);

	const Position *ptr = e.try_get<Position>();
	test_int(ptr->x, 11);
	test_int(ptr->y, 21);
}

void Query_query_iter_w_func_ptr(void) {
	flecs::world w;
	RegisterTestTypeComponents(w);
	invoked_count = 0;

	auto e = w.entity().set<Position>({10, 20});

	auto q = w.query<Position>();

	q.run(RunFunc);

	test_int(invoked_count, 1);

	const Position *ptr = e.try_get<Position>();
	test_int(ptr->x, 11);
	test_int(ptr->y, 21);
}

void Query_query_each_w_func_no_ptr(void) {
	flecs::world w;
	RegisterTestTypeComponents(w);
	invoked_count = 0;

	auto e = w.entity().set<Position>({10, 20});

	auto q = w.query<Position>();

	q.each(EachFunc);

	test_int(invoked_count, 1);

	const Position *ptr = e.try_get<Position>();
	test_int(ptr->x, 11);
	test_int(ptr->y, 21);
}

void Query_query_iter_w_func_no_ptr(void) {
	flecs::world w;
	RegisterTestTypeComponents(w);
	invoked_count = 0;

	auto e = w.entity().set<Position>({10, 20});

	auto q = w.query<Position>();

	q.run(RunFunc);

	test_int(invoked_count, 1);

	const Position *ptr = e.try_get<Position>();
	test_int(ptr->x, 11);
	test_int(ptr->y, 21);
}

void Query_query_each_w_iter(void) {
	flecs::world w;
	RegisterTestTypeComponents(w);

	auto e1 = w.entity(); e1.set<Self>({e1});
	e1.set<Position>({10, 20});
	auto e2 = w.entity(); e2.set<Self>({e2});
	e2.set<Position>({20, 30});

	auto q = w.query<Self, Position>();

	int32_t invoked = 0;
	q.each([&](flecs::iter& it, int32_t i, Self& s, Position& p) {
		test_int(it.count(), 2);
		test_int(it.entity(i), s.value);
		p.x ++;
		p.y ++;
		invoked ++;
	});

	test_int(invoked, 2);

	const Position *ptr = e1.try_get<Position>();
	test_int(ptr->x, 11);
	test_int(ptr->y, 21);

	ptr = e2.try_get<Position>();
	test_int(ptr->x, 21);
	test_int(ptr->y, 31);
}

void Query_field_at_from_each_w_iter(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	flecs::entity e1 = ecs.entity()
		.set<Position>({10, 20})
		.set<Velocity>({1, 2});

	flecs::entity e2 = ecs.entity()
		.set<Position>({20, 30})
		.set<Velocity>({3, 4});

	auto q = ecs.query_builder<Position>()
		.with<Velocity>().inout()
		.build();

	int32_t count = 0;

	q.each([&](flecs::iter& it, size_t row, Position& p) {
		Velocity* v = static_cast<Velocity*>(it.field_at(1, row));
		if (it.entity(row) == e1) {
			test_int(v->x, 1);
			test_int(v->y, 2);
			count ++;
		} else if (it.entity(row) == e2) {
			test_int(v->x, 3);
			test_int(v->y, 4);
			count ++;
		}
	});

	test_int(count, 2);
}

void Query_field_at_T_from_each_w_iter(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	flecs::entity e1 = ecs.entity()
		.set<Position>({10, 20})
		.set<Velocity>({1, 2});

	flecs::entity e2 = ecs.entity()
		.set<Position>({20, 30})
		.set<Velocity>({3, 4});

	auto q = ecs.query_builder<Position>()
		.with<Velocity>().inout()
		.build();

	int32_t count = 0;

	q.each([&](flecs::iter& it, size_t row, Position& p) {
		Velocity& v = it.field_at<Velocity>(1, row);
		if (it.entity(row) == e1) {
			test_int(v.x, 1);
			test_int(v.y, 2);
			count ++;
		} else if (it.entity(row) == e2) {
			test_int(v.x, 3);
			test_int(v.y, 4);
			count ++;
		}
	});

	test_int(count, 2);
}

void Query_field_at_const_T_from_each_w_iter(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	flecs::entity e1 = ecs.entity()
		.set<Position>({10, 20})
		.set<Velocity>({1, 2});

	flecs::entity e2 = ecs.entity()
		.set<Position>({20, 30})
		.set<Velocity>({3, 4});

	auto q = ecs.query_builder<Position>()
		.with<Velocity>().inout()
		.build();

	int32_t count = 0;

	q.each([&](flecs::iter& it, size_t row, Position& p) {
		const Velocity& v = it.field_at<const Velocity>(1, row);
		if (it.entity(row) == e1) {
			test_int(v.x, 1);
			test_int(v.y, 2);
			count ++;
		} else if (it.entity(row) == e2) {
			test_int(v.x, 3);
			test_int(v.y, 4);
			count ++;
		}
	});

	test_int(count, 2);
}

void Query_field_at_from_each_w_iter_w_base_type(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);
	ecs.component<VelocityDerived>();

	flecs::entity e1 = ecs.entity()
		.set<Position>({10, 20})
		.set<VelocityDerived>({1, 2, 3});

	flecs::entity e2 = ecs.entity()
		.set<Position>({20, 30})
		.set<VelocityDerived>({3, 4, 5});

	auto q = ecs.query_builder<Position>()
		.with<VelocityDerived>().inout()
		.build();

	int32_t count = 0;

	q.each([&](flecs::iter& it, size_t row, Position& p) {
		Velocity* v = static_cast<Velocity*>(it.field_at(1, row));
		if (it.entity(row) == e1) {
			test_int(v->x, 1);
			test_int(v->y, 2);
			count ++;
		} else if (it.entity(row) == e2) {
			test_int(v->x, 3);
			test_int(v->y, 4);
			count ++;
		}
	});

	test_int(count, 2);
}


void Query_change_tracking(void) {
	flecs::world w;
	RegisterTestTypeComponents(w);

	auto qw = w.query<Position>();
	auto qr = w.query_builder<const Position>()
		.detect_changes()
		.cached()
		.build();

	auto e1 = w.entity().add<Tag>().set<Position>({10, 20});
	w.entity().set<Position>({20, 30});

	test_bool(qr.changed(), true);
	qr.run([](flecs::iter &it) { while (it.next()) {} });
	test_bool(qr.changed(), false);

	int32_t count = 0, change_count = 0;

	qw.run([&](flecs::iter& it) {
		while (it.next()) {
			test_int(it.count(), 1);

			count ++;

			if (it.entity(0) == e1) {
				it.skip();
				continue;
			}

			change_count ++;
		}
	});

	test_int(count, 2);
	test_int(change_count, 1);

	count = 0;
	change_count = 0;

	test_bool(qr.changed(), true);

	qr.run([&](flecs::iter& it) {
		while (it.next()) {
			test_int(it.count(), 1);

			count ++;

			if (it.entity(0) == e1) {
				test_bool(it.changed(), false);
			} else {
				test_bool(it.changed(), true);
				change_count ++;
			}
		}
	});

	test_int(count, 2);
	test_int(change_count, 1);
}

void Query_not_w_write(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	struct A {};
	struct B {};
	ecs.component<A>();
	ecs.component<B>();

	auto q = ecs.query_builder()
		.with<A>()
		.with<B>().oper(flecs::Not).write()
		.build();

	auto e = ecs.entity().add<A>();

	int32_t count = 0;
	ecs.defer([&] {
		q.each([&](flecs::entity e) {
			e.add<B>();
			count ++;
		});
	});

	test_int(1, count);
	test_assert(e.has<B>());

	q.each([&](flecs::entity e) {
		count ++;
	});

	test_int(1, count);
}

void Query_instanced_nested_query_w_iter(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	ecs.component<Mass>().add(flecs::Singleton);

	flecs::query<> q1 = ecs.query_builder()
		.with<Position>()
		.with<Mass>().inout()
		.build();

	flecs::query<> q2 = ecs.query_builder()
		.with<Velocity>()
		.build();

	ecs.add<Mass>();
	ecs.entity().add<Velocity>();
	ecs.entity().add<Position>();
	ecs.entity().add<Position>();

	int32_t count = 0;

	q2.run([&](flecs::iter& it_2) {
		while (it_2.next()) {
			q1.iter(it_2).run([&](flecs::iter& it_1) {
				while (it_1.next()) {
					test_int(it_1.count(), 2);
					count += it_1.count();
				}
			});
		}
	});

	test_int(count, 2);
}

void Query_instanced_nested_query_w_entity(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	ecs.component<Mass>().add(flecs::Singleton);

	flecs::query<> q1 = ecs.query_builder()
		.with<Position>()
		.with<Mass>().inout()
		.build();

	flecs::query<> q2 = ecs.query_builder()
		.with<Velocity>()
		.build();

	ecs.add<Mass>();
	ecs.entity().add<Velocity>();
	ecs.entity().add<Position>();
	ecs.entity().add<Position>();

	int32_t count = 0;

	q2.each([&](flecs::entity e_2) {
		q1.iter(e_2).run([&](flecs::iter& it_1) {
			while (it_1.next()) {
				test_int(it_1.count(), 2);
				count += it_1.count();
			}
		});
	});

	test_int(count, 2);
}

void Query_instanced_nested_query_w_world(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	ecs.component<Mass>().add(flecs::Singleton);

	flecs::query<> q1 = ecs.query_builder()
		.with<Position>()
		.with<Mass>().inout()
		.build();

	flecs::query<> q2 = ecs.query_builder()
		.with<Velocity>()
		.build();

	ecs.add<Mass>();
	ecs.entity().add<Velocity>();
	ecs.entity().add<Position>();
	ecs.entity().add<Position>();

	int32_t count = 0;

	q2.run([&](flecs::iter& it_2) {
		while (it_2.next()) {
			q1.iter(it_2.world()).run([&](flecs::iter& it_1) {
				while (it_1.next()) {
					test_int(it_1.count(), 2);
					count += it_1.count();
				}
			});
		}
	});

	test_int(count, 2);
}

void Query_captured_query(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	flecs::query<Position> q = ecs.query<Position>();
	flecs::entity e_1 = ecs.entity().set<Position>({10, 20});

	[=]() {
		int count = 0;
		q.each([&](flecs::entity e, Position& p) {
			test_assert(e == e_1);
			test_int(p.x, 10);
			test_int(p.y, 20);
			count ++;
		});
		test_int(count, 1);
	}();
}

void Query_page_iter_captured_query(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	flecs::query<Position> q = ecs.query<Position>();
	/* flecs::entity e_1 = */ ecs.entity().set<Position>({10, 20});
	flecs::entity e_2 = ecs.entity().set<Position>({20, 30});
	/* flecs::entity e_3 = */ ecs.entity().set<Position>({10, 20});

	[=]() {
		int count = 0;
		q.iter().page(1, 1).each([&](flecs::entity e, Position& p) {
			test_assert(e == e_2);
			test_int(p.x, 20);
			test_int(p.y, 30);
			count ++;
		});
		test_int(count, 1);
	}();
}

void Query_worker_iter_captured_query(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	flecs::query<Position> q = ecs.query<Position>();
	/* flecs::entity e_1 = */ ecs.entity().set<Position>({10, 20});
	flecs::entity e_2 = ecs.entity().set<Position>({20, 30});
	/* flecs::entity e_3 = */ ecs.entity().set<Position>({10, 20});

	[=]() {
		int count = 0;
		q.iter().worker(1, 3).each([&](flecs::entity e, Position& p) {
			test_assert(e == e_2);
			test_int(p.x, 20);
			test_int(p.y, 30);
			count ++;
		});
		test_int(count, 1);
	}();
}

void Query_set_group_captured_query(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	flecs::entity Rel = ecs.entity();
	flecs::entity TgtA = ecs.entity();
	flecs::entity TgtB = ecs.entity();

	flecs::query<Position> q = ecs.query_builder<Position>()
		.group_by(Rel)
		.build();

	/* flecs::entity e_1 = */ ecs.entity().set<Position>({10, 20}).add(Rel, TgtA);
	flecs::entity e_2 = ecs.entity().set<Position>({20, 30}).add(Rel, TgtB);

	[=]() {
		int count = 0;
		q.set_group(TgtB).each([&](flecs::entity e, Position& p) {
			test_assert(e == e_2);
			test_int(p.x, 20);
			test_int(p.y, 30);
			count ++;
		});
		test_int(count, 1);
	}();
}

void Query_set_var_captured_query(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	flecs::entity Rel = ecs.entity();
	flecs::entity TgtA = ecs.entity();
	flecs::entity TgtB = ecs.entity();

	flecs::query<Position> q = ecs.query_builder<Position>()
		.with(Rel, "$var")
		.build();

	/* flecs::entity e_1 = */ ecs.entity().set<Position>({10, 20}).add(Rel, TgtA);
	flecs::entity e_2 = ecs.entity().set<Position>({20, 30}).add(Rel, TgtB);

	[=]() {
		int count = 0;
		q.set_var("var", TgtB).each([&](flecs::entity e, Position& p) {
			test_assert(e == e_2);
			test_int(p.x, 20);
			test_int(p.y, 30);
			count ++;
		});
		test_int(count, 1);
	}();
}


void Query_set_var_id_captured_query(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	flecs::entity Rel = ecs.entity();
	flecs::entity TgtA = ecs.entity();
	flecs::entity TgtB = ecs.entity();

	flecs::query<Position> q = ecs.query_builder<Position>()
		.with(Rel, "$var")
		.build();

	int var = q.find_var("var");
	test_assert(var != -1);

	/* flecs::entity e_1 = */ ecs.entity().set<Position>({10, 20}).add(Rel, TgtA);
	flecs::entity e_2 = ecs.entity().set<Position>({20, 30}).add(Rel, TgtB);

	[=]() {
		int count = 0;
		q.set_var(var, TgtB).each([&](flecs::entity e, Position& p) {
			test_assert(e == e_2);
			test_int(p.x, 20);
			test_int(p.y, 30);
			count ++;
		});
		test_int(count, 1);
	}();
}

void Query_iter_entities(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	auto e1 = ecs.entity().set<Position>({10, 20});
	auto e2 = ecs.entity().set<Position>({10, 20});
	auto e3 = ecs.entity().set<Position>({10, 20});

	ecs.query<Position>()
		.run([&](flecs::iter& it) {
			while (it.next()) {
				test_int(it.count(), 3);

				auto entities = it.entities();
				test_assert(entities[0] == e1);
				test_assert(entities[1] == e2);
				test_assert(entities[2] == e3);
			}
		});
}

void Query_iter_get_pair_w_id(void) {
	flecs::world ecs;

	flecs::entity Rel = ecs.entity();
	flecs::entity Tgt = ecs.entity();
	flecs::entity e = ecs.entity().add(Rel, Tgt);

	flecs::query<> q = ecs.query_builder()
		.with(Rel, flecs::Wildcard)
		.build();

	int32_t count = 0;

	q.each([&](flecs::iter& it, size_t i) {
		test_bool(true, it.id(0).is_pair());
		test_assert(it.id(0).first() == Rel);
		test_assert(it.id(0).second() == Tgt);
		test_assert(e == it.entity(i));
		count ++;
	});

	test_int(count, 1);
}

void Query_query_from_entity(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	flecs::entity qe = ecs.entity();
	flecs::query<Position, Velocity> q1 = ecs.query_builder<Position, Velocity>(qe)
		.build();

	/* flecs::entity e1 = */ ecs.entity().add<Position>();
	flecs::entity e2 = ecs.entity().add<Position>().add<Velocity>();

	int32_t count = 0;
	q1.each([&](flecs::entity e, Position&, Velocity&) {
		count ++;
		test_assert(e == e2);
	});
	test_int(count, 1);

	flecs::query<> q2 = ecs.query(qe);
	q2.each([&](flecs::entity e) {
		count ++;
		test_assert(e == e2);
	});
	test_int(count, 2);
}

void Query_query_from_entity_name(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	flecs::query<Position, Velocity> q1 = ecs.query_builder<Position, Velocity>("qe")
		.build();

	/* flecs::entity e1 = */ ecs.entity().add<Position>();
	flecs::entity e2 = ecs.entity().add<Position>().add<Velocity>();

	int32_t count = 0;
	q1.each([&](flecs::entity e, Position&, Velocity&) {
		count ++;
		test_assert(e == e2);
	});
	test_int(count, 1);

	flecs::query<> q2 = ecs.query("qe");
	q2.each([&](flecs::entity e) {
		count ++;
		test_assert(e == e2);
	});
	test_int(count, 2);
}

void Query_run_w_iter_fini(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	flecs::query<Position> q = ecs.query<Position>();

	int32_t count = 0;
	q.run([&](flecs::iter& it) {
		it.fini();
		count ++;
	});

	test_int(count, 1);

	// should be no leakage assert
}

void Query_run_w_iter_fini_interrupt(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	struct Foo {};
	struct Bar {};
	struct Hello {};
	ecs.component<Foo>();
	ecs.component<Bar>();
	ecs.component<Hello>();

	flecs::entity e1 = ecs.entity()
		.set<Position>({10, 20})
		.add<Foo>();
	/* flecs::entity e2 = */ ecs.entity()
		.set<Position>({10, 20})
		.add<Bar>();
	/* flecs::entity e3 = */ ecs.entity()
		.set<Position>({10, 20})
		.add<Hello>();

	flecs::query<Position> q = ecs.query<Position>();

	int32_t count = 0;
	q.run([&](flecs::iter& it) {
		test_bool(true, it.next());
		test_int(1, it.count());
		test_uint(e1, it.entity(0));

		test_bool(true, it.next());
		count ++;
		it.fini();
	});

	test_int(count, 1);
}

void Query_run_w_iter_fini_empty(void) {
	flecs::world ecs;
	RegisterTestTypeComponents(ecs);

	flecs::query<Position> q = ecs.query<Position>();

	int32_t count = 0;
	q.run([&](flecs::iter& it) {
		count ++;
		it.fini();
	});

	test_int(count, 1);
}

void Query_run_w_iter_fini_no_query(void) {
	flecs::world ecs;

	flecs::query<> q = ecs.query();

	int32_t count = 0;
	q.run([&](flecs::iter& it) {
		count ++;
		it.fini();
	});

	test_int(count, 1);
}

void Query_add_to_match_from_staged_query(void) {
	flecs::world ecs;

	ecs.component<Position>();
	ecs.component<Velocity>();

	flecs::entity e = ecs.entity().add<Position>();

	flecs::world stage = ecs.get_stage(0);

	ecs.readonly_begin(false);

	stage.query<Position>()
		.each([](flecs::entity e, Position&) {
			e.add<Velocity>();
			test_assert(!e.has<Velocity>());
		});

	ecs.readonly_end();

	test_assert(e.has<Position>());
	test_assert(e.has<Velocity>());
}

void Query_add_to_match_from_staged_query_readonly_threaded(void) {
	flecs::world ecs;

	ecs.component<Position>();
	ecs.component<Velocity>();

	flecs::entity e = ecs.entity().add<Position>();

	flecs::world stage = ecs.get_stage(0);

	ecs.readonly_begin(true);

	stage.query<Position>()
		.each([](flecs::entity e, Position&) {
			e.add<Velocity>();
			test_assert(!e.has<Velocity>());
		});

	ecs.readonly_end();

	test_assert(e.has<Position>());
	test_assert(e.has<Velocity>());
}

void Query_empty_tables_each(void) {
	flecs::world world;

	world.component<Position>();
	world.component<Velocity>();
	world.component<Tag>();

	auto e1 = world.entity()
		.set<Position>({10, 20})
		.set<Velocity>({1, 2});

	auto e2 = world.entity()
		.set<Position>({20, 30})
		.set<Velocity>({2, 3});

	e2.add<Tag>();
	e2.remove<Tag>();

	auto q = world.query_builder<Position, Velocity>()
		.query_flags(EcsQueryMatchEmptyTables)
		.build();

	q.each([](Position& p, Velocity& v) {
		p.x += v.x;
		p.y += v.y;
	});

	{
		const Position *p = e1.try_get<Position>();
		test_int(p->x, 11);
		test_int(p->y, 22);
	}
	{
		const Position *p = e2.try_get<Position>();
		test_int(p->x, 22);
		test_int(p->y, 33);
	}
}

void Query_empty_tables_each_w_entity(void) {
	flecs::world world;

	world.component<Position>();
	world.component<Velocity>();
	world.component<Tag>();

	auto e1 = world.entity()
		.set<Position>({10, 20})
		.set<Velocity>({1, 2});

	auto e2 = world.entity()
		.set<Position>({20, 30})
		.set<Velocity>({2, 3});

	e2.add<Tag>();
	e2.remove<Tag>();

	auto q = world.query_builder<Position, Velocity>()
		.query_flags(EcsQueryMatchEmptyTables)
		.build();

	q.each([](flecs::entity e, Position& p, Velocity& v) {
		p.x += v.x;
		p.y += v.y;
	});

	{
		const Position *p = e1.try_get<Position>();
		test_int(p->x, 11);
		test_int(p->y, 22);
	}
	{
		const Position *p = e2.try_get<Position>();
		test_int(p->x, 22);
		test_int(p->y, 33);
	}
}

void Query_empty_tables_each_w_iter(void) {
	flecs::world world;

	world.component<Position>();
	world.component<Velocity>();
	world.component<Tag>();

	auto e1 = world.entity()
		.set<Position>({10, 20})
		.set<Velocity>({1, 2});

	auto e2 = world.entity()
		.set<Position>({20, 30})
		.set<Velocity>({2, 3});

	e2.add<Tag>();
	e2.remove<Tag>();

	auto q = world.query_builder<Position, Velocity>()
		.query_flags(EcsQueryMatchEmptyTables)
		.build();

	q.each([](flecs::iter&, size_t, Position& p, Velocity& v) {
		p.x += v.x;
		p.y += v.y;
	});

	{
		const Position *p = e1.try_get<Position>();
		test_int(p->x, 11);
		test_int(p->y, 22);
	}
	{
		const Position *p = e2.try_get<Position>();
		test_int(p->x, 22);
		test_int(p->y, 33);
	}
}

void Query_pair_with_variable_src(void) {
	flecs::world world;

	struct Rel {};
	struct ThisComp { int x; };
	struct OtherComp { int x; };

	world.component<Rel>();
	world.component<ThisComp>();
	world.component<OtherComp>();

	auto other = world.entity()
		.set(OtherComp{10});

	for (int i = 0; i < 3; ++i) {
		world.entity()
			.set(ThisComp{i})
			.add<Rel>(other);
	}

	auto q = world.query_builder<const Rel, const ThisComp, const OtherComp>()
		.term_at(0).second("$other")
		.term_at(2).src("$other")
		.build();

	size_t isPresent = 0;
	q.each([&isPresent](const Rel& rel, const ThisComp& thisComp, const OtherComp& otherComp) {
		isPresent |= (1ULL << thisComp.x);
		test_int(otherComp.x, 10);
	});

	test_int(isPresent, 7);
}

void Query_pair_with_variable_src_no_row_fields(void) {
	flecs::world world;

	struct Rel { int dummy; }; // make sure this isn't a tag, so that the query contains no row field
	struct ThisComp { int x; };
	struct OtherComp { int x; };

	world.component<Rel>();
	world.component<ThisComp>();
	world.component<OtherComp>();

	auto other = world.entity()
		.set(OtherComp{0});

	// Guarantee we don't luckily hit zero if we read the wrong component
	world.entity()
		.set(OtherComp{1});

	for (int i = 0; i < 3; ++i) {
		world.entity()
			.set(ThisComp{i})
			.add<Rel>(other);
	}

	auto q = world.query_builder<const Rel, const ThisComp, const OtherComp>()
		.term_at(0).second("$other")
		.term_at(2).src("$other")
		.build();

	size_t isPresent = 0;
	q.each([&isPresent](const Rel& rel, const ThisComp& thisComp, const OtherComp& otherComp) {
		isPresent |= (1ULL << thisComp.x);
		test_int(otherComp.x, 0);
	});

	test_int(isPresent, 7);
}

void Query_iter_targets(void) {
	flecs::world world;

	flecs::entity Likes = world.entity();
	flecs::entity pizza = world.entity();
	flecs::entity salad = world.entity();
	flecs::entity alice = world.entity().add(Likes, pizza).add(Likes, salad);

	auto q = world.query_builder()
		.with(Likes, flecs::Any)
		.build();

	int count = 0, tgt_count = 0;

	q.each([&](flecs::iter& it, size_t row) {
		flecs::entity e = it.entity(row);
		test_assert(e == alice);

		it.targets(0, [&](flecs::entity tgt) {
			if (tgt_count == 0) {
				test_assert(tgt == pizza);
			}
			if (tgt_count == 1) {
				test_assert(tgt == salad);
			}
			tgt_count ++;
		});

		count ++;
	});

	test_int(count, 1);
	test_int(tgt_count, 2);
}

void Query_iter_targets_2nd_field(void) {
	flecs::world world;
	RegisterTestTypeComponents(world);

	flecs::entity Likes = world.entity();
	flecs::entity pizza = world.entity();
	flecs::entity salad = world.entity();
	flecs::entity alice = world.entity()
		.add<Position>()
		.add(Likes, pizza).add(Likes, salad);

	auto q = world.query_builder()
		.with<Position>()
		.with(Likes, flecs::Any)
		.build();

	int count = 0, tgt_count = 0;

	q.each([&](flecs::iter& it, size_t row) {
		flecs::entity e = it.entity(row);
		test_assert(e == alice);

		it.targets(1, [&](flecs::entity tgt) {
			if (tgt_count == 0) {
				test_assert(tgt == pizza);
			}
			if (tgt_count == 1) {
				test_assert(tgt == salad);
			}
			tgt_count ++;
		});

		count ++;
	});

	test_int(count, 1);
	test_int(tgt_count, 2);
}

void Query_copy_operators(void) {
	flecs::world world{};
	RegisterTestTypeComponents(world);

	flecs::query<> q = world.query_builder()
		.with<Position>()
		.build();

	flecs::query<> copyCtor{q};
	flecs::query<> copyAssign{};
	copyAssign = q;

	test_assert(copyAssign.c_ptr() == q.c_ptr());

	flecs::query<> defaultInit{};
	flecs::query<> copyCtorDefault{defaultInit};
	copyAssign = defaultInit;

	test_assert(copyAssign.c_ptr() == defaultInit.c_ptr());
}

END_DEFINE_SPEC(FFlecsQueryTestsSpec);

/*"id": "Query",
            "testcases": [
                "term_each_component",
                "term_each_tag",
                "term_each_id",
                "term_each_pair_type",
                "term_each_pair_id",
                "term_each_pair_relation_wildcard",
                "term_each_pair_object_wildcard",
                "term_get_id",
                "term_get_subj",
                "term_get_pred",
                "term_get_obj",
                "set_this_var",
                "run",
                "run_const",
                "run_shared",
                "run_optional",
                "run_sparse",
                "run_sparse_w_with",
				"run_dont_fragment",
				"run_dont_fragment_w_with",
				"run_dont_fragment_add",
				"run_dont_fragment_add_remove",
				"run_dont_fragment_set",
                "each",
                "each_const",
                "each_shared",
                "each_optional",
                "each_sparse",
                "each_sparse_w_with",
                "each_sparse_many",
				"each_dont_fragment",
				"each_dont_fragment_w_with",
				"each_dont_fragment_many",
				"each_dont_fragment_add",
				"each_dont_fragment_add_remove",
				"each_dont_fragment_set",
                "signature",
                "signature_const",
                "signature_shared",
                "signature_optional",
                "query_single_pair",
                "tag_w_each",
                "shared_tag_w_each",
                "sort_by",
                "changed",
                "default_ctor",
                "default_ctor_no_assign",
                "expr_w_template",
                "query_type_w_template",
                "compare_term_id",
                "inspect_terms",
                "inspect_terms_w_each",
                "inspect_terms_w_expr",
                "comp_to_str",
                "pair_to_str",
                "oper_not_to_str",
                "oper_optional_to_str",
                "oper_or_to_str",
                "each_pair_type",
                "iter_pair_type",
                "term_pair_type",
                "each_no_entity_1_comp",
                "each_no_entity_2_comps",
                "iter_no_comps_1_comp",
                "iter_no_comps_2_comps",
                "iter_no_comps_no_comps",
                "each_pair_object",
                "iter_pair_object",
                "iter_query_in_system",
                "iter_type",
                "instanced_query_w_singleton_each",
                "instanced_query_w_base_each",
                "instanced_query_w_singleton_iter",
                "instanced_query_w_base_iter",
                "query_each_from_component",
                "query_iter_from_component",
                "query_each_w_func_ptr",
                "query_iter_w_func_ptr",
                "query_each_w_func_no_ptr",
                "query_iter_w_func_no_ptr",
                "query_each_w_iter",
                "each_w_iter_no_this",
                "field_at_from_each_w_iter",
                "field_at_T_from_each_w_iter",
                "field_at_const_T_from_each_w_iter",
                "field_at_from_each_w_iter_w_base_type",
                "change_tracking",
                "not_w_write",
                "get_first",
                "get_count_direct",
                "get_is_true_direct",
                "get_first_direct",
                "each_w_no_this",
                "named_query",
                "named_scoped_query",
                "instanced_nested_query_w_iter",
                "instanced_nested_query_w_entity",
                "instanced_nested_query_w_world",
                "captured_query",
                "page_iter_captured_query",
                "worker_iter_captured_query",
                "set_group_captured_query",
                "set_var_captured_query",
                "set_var_id_captured_query",
                "iter_entities",
                "iter_get_pair_w_id",
                "find",
                "find_not_found",
                "find_w_entity",
                "find_w_match_empty_tables",
                "find_w_entity_w_match_empty_tables",
                "optional_pair_term",
                "empty_tables_each",
                "empty_tables_each_w_entity",
                "empty_tables_each_w_iter",
                "query_from_entity",
                "query_from_entity_name",
                "run_w_iter_fini",
                "run_w_iter_fini_interrupt",
                "run_w_iter_fini_empty",
                "run_w_iter_fini_no_query",
                "add_to_match_from_staged_query",
                "add_to_match_from_staged_query_readonly_threaded",
                "pair_with_variable_src",
                "pair_with_variable_src_no_row_fields",
                "iter_targets",
                "iter_targets_2nd_field",
                "copy_operators"
            ]*/

void FFlecsQueryTestsSpec::Define()
{
	It("Query_term_each_component", [&]() { Query_term_each_component(); });
	It("Query_term_each_tag", [&]() { Query_term_each_tag(); });
	It("Query_term_each_id", [&]() { Query_term_each_id(); });
	It("Query_term_each_pair_type", [&]() { Query_term_each_pair_type(); });
	It("Query_term_each_pair_id", [&]() { Query_term_each_pair_id(); });
	It("Query_term_each_pair_relation_wildcard", [&]() { Query_term_each_pair_relation_wildcard(); });
	It("Query_term_each_pair_object_wildcard", [&]() { Query_term_each_pair_object_wildcard(); });
	It("Query_term_get_id", [&]() { Query_term_get_id(); });
	It("Query_term_get_subj", [&]() { Query_term_get_subj(); });
	It("Query_term_get_pred", [&]() { Query_term_get_pred(); });
	It("Query_term_get_obj", [&]() { Query_term_get_obj(); });
	It("Query_set_this_var", [&]() { Query_set_this_var(); });
	It("Query_run", [&]() { Query_run(); });
	It("Query_run_const", [&]() { Query_run_const(); });
	It("Query_run_shared", [&]() { Query_run_shared(); });
	It("Query_run_optional", [&]() { Query_run_optional(); });
	It("Query_run_sparse", [&]() { Query_run_sparse(); });
	It("Query_run_sparse_w_with", [&]() { Query_run_sparse_w_with(); });
	It("Query_run_dont_fragment", [&]() { Query_run_dont_fragment(); });
	It("Query_run_dont_fragment_w_with", [&]() { Query_run_dont_fragment_w_with(); });
	It("Query_run_dont_fragment_add", [&]() { Query_run_dont_fragment_add(); });
	It("Query_run_dont_fragment_add_remove", [&]() { Query_run_dont_fragment_add_remove(); });
	It("Query_run_dont_fragment_set", [&]() { Query_run_dont_fragment_set(); });
	It("Query_each", [&]() { Query_each(); });
	It("Query_each_const", [&]() { Query_each_const(); });
	It("Query_each_shared", [&]() { Query_each_shared(); });
	It("Query_each_optional", [&]() { Query_each_optional(); });
	It("Query_each_sparse", [&]() { Query_each_sparse(); });
	It("Query_each_sparse_w_with", [&]() { Query_each_sparse_w_with(); });
	It("Query_each_sparse_many", [&]() { Query_each_sparse_many(); });
	It("Query_each_dont_fragment", [&]() { Query_each_dont_fragment(); });
	It("Query_each_dont_fragment_w_with", [&]() { Query_each_dont_fragment_w_with(); });
	It("Query_each_dont_fragment_many", [&]() { Query_each_dont_fragment_many(); });
	It("Query_each_dont_fragment_add", [&]() { Query_each_dont_fragment_add(); });
	It("Query_each_dont_fragment_add_remove", [&]() { Query_each_dont_fragment_add_remove(); });
	It("Query_each_dont_fragment_set", [&]() { Query_each_dont_fragment_set(); });
	It("Query_signature", [&]() { Query_signature(); });
	It("Query_signature_const", [&]() { Query_signature_const(); });
	It("Query_signature_shared", [&]() { Query_signature_shared(); });
	It("Query_signature_optional", [&]() { Query_signature_optional(); });
	It("Query_query_single_pair", [&]() { Query_query_single_pair(); });
	It("Query_tag_w_each", [&]() { Query_tag_w_each(); });
	It("Query_shared_tag_w_each", [&]() { Query_shared_tag_w_each(); });
	It("Query_sort_by", [&]() { Query_sort_by(); });
	It("Query_changed", [&]() { Query_changed(); });
	It("Query_default_ctor", [&]() { Query_default_ctor(); });
	It("Query_default_ctor_no_assign", [&]() { Query_default_ctor_no_assign(); });
	It("Query_expr_w_template", [&]() { Query_expr_w_template(); });
	It("Query_query_type_w_template", [&]() { Query_query_type_w_template(); });
	It("Query_compare_term_id", [&]() { Query_compare_term_id(); });
	It("Query_inspect_terms", [&]() { Query_inspect_terms(); });
	It("Query_inspect_terms_w_each", [&]() { Query_inspect_terms_w_each(); });
	It("Query_inspect_terms_w_expr", [&]() { Query_inspect_terms_w_expr(); });
	It("Query_comp_to_str", [&]() { Query_comp_to_str(); });
	It("Query_pair_to_str", [&]() { Query_pair_to_str(); });
	It("Query_oper_not_to_str", [&]() { Query_oper_not_to_str(); });
	It("Query_oper_optional_to_str", [&]() { Query_oper_optional_to_str(); });
	It("Query_oper_or_to_str", [&]() { Query_oper_or_to_str(); });
	It("Query_each_pair_type", [&]() { Query_each_pair_type(); });
	It("Query_iter_pair_type", [&]() { Query_iter_pair_type(); });
	It("Query_term_pair_type", [&]() { Query_term_pair_type(); });
	It("Query_each_no_entity_1_comp", [&]() { Query_each_no_entity_1_comp(); });
	It("Query_each_no_entity_2_comps", [&]() { Query_each_no_entity_2_comps(); });
	It("Query_iter_no_comps_1_comp", [&]() { Query_iter_no_comps_1_comp(); });
	It("Query_iter_no_comps_2_comps", [&]() { Query_iter_no_comps_2_comps(); });
	It("Query_iter_no_comps_no_comps", [&]() { Query_iter_no_comps_no_comps(); });
	It("Query_each_pair_object", [&]() { Query_each_pair_object(); });
	It("Query_iter_pair_object", [&]() { Query_iter_pair_object(); });
	It("Query_iter_query_in_system", [&]() { Query_iter_query_in_system(); });
	It("Query_iter_type", [&]() { Query_iter_type(); });
	It("Query_instanced_query_w_singleton_each", [&]() { Query_instanced_query_w_singleton_each(); });
	It("Query_instanced_query_w_base_each", [&]() { Query_instanced_query_w_base_each(); });
	It("Query_instanced_query_w_singleton_iter", [&]() { Query_instanced_query_w_singleton_iter(); });
	It("Query_instanced_query_w_base_iter", [&]() { Query_instanced_query_w_base_iter(); });
	It("Query_query_each_from_component", [&]() { Query_query_each_from_component(); });
	It("Query_query_iter_from_component", [&]() { Query_query_iter_from_component(); });
	It("Query_query_each_w_func_ptr", [&]() { Query_query_each_w_func_ptr(); });
	It("Query_query_iter_w_func_ptr", [&]() { Query_query_iter_w_func_ptr(); });
	It("Query_query_each_w_func_no_ptr", [&]() { Query_query_each_w_func_no_ptr(); });
	It("Query_query_iter_w_func_no_ptr", [&]() { Query_query_iter_w_func_no_ptr(); });
	It("Query_query_each_w_iter", [&]() { Query_query_each_w_iter(); });
	It("Query_each_w_iter_no_this", [&]() { Query_each_w_iter_no_this(); });
	It("Query_field_at_from_each_w_iter", [&]() { Query_field_at_from_each_w_iter(); });
	It("Query_field_at_T_from_each_w_iter", [&]() { Query_field_at_T_from_each_w_iter(); });
	It("Query_field_at_const_T_from_each_w_iter", [&]() { Query_field_at_const_T_from_each_w_iter(); });
	It("Query_field_at_from_each_w_iter_w_base_type", [&]() { Query_field_at_from_each_w_iter_w_base_type(); });
	It("Query_change_tracking", [&]() { Query_change_tracking(); });
	It("Query_not_w_write", [&]() { Query_not_w_write(); });
	It("Query_get_first", [&]() { Query_get_first(); });
	It("Query_get_count_direct", [&]() { Query_get_count_direct(); });
	It("Query_get_is_true_direct", [&]() { Query_get_is_true_direct(); });
	It("Query_get_first_direct", [&]() { Query_get_first_direct(); });
	It("Query_each_w_no_this", [&]() { Query_each_w_no_this(); });
	It("Query_named_query", [&]() { Query_named_query(); });
	It("Query_named_scoped_query", [&]() { Query_named_scoped_query(); });
	It("Query_instanced_nested_query_w_iter", [&]() { Query_instanced_nested_query_w_iter(); });
	It("Query_instanced_nested_query_w_entity", [&]() { Query_instanced_nested_query_w_entity(); });
	It("Query_instanced_nested_query_w_world", [&]() { Query_instanced_nested_query_w_world(); });
	It("Query_captured_query", [&]() { Query_captured_query(); });
	It("Query_page_iter_captured_query", [&]() { Query_page_iter_captured_query(); });
	It("Query_worker_iter_captured_query", [&]() { Query_worker_iter_captured_query(); });
	It("Query_set_group_captured_query", [&]() { Query_set_group_captured_query(); });
	It("Query_set_var_captured_query", [&]() { Query_set_var_captured_query(); });
	It("Query_set_var_id_captured_query", [&]() { Query_set_var_id_captured_query(); });
	It("Query_iter_entities", [&]() { Query_iter_entities(); });
	It("Query_iter_get_pair_w_id", [&]() { Query_iter_get_pair_w_id(); });
	It("Query_find", [&]() { Query_find(); });
	It("Query_find_not_found", [&]() { Query_find_not_found(); });
	It("Query_find_w_entity", [&]() { Query_find_w_entity(); });
	It("Query_find_w_match_empty_tables", [&]() { Query_find_w_match_empty_tables(); });
	It("Query_find_w_entity_w_match_empty_tables", [&]() { Query_find_w_entity_w_match_empty_tables(); });
	It("Query_optional_pair_term", [&]() { Query_optional_pair_term(); });
	It("Query_empty_tables_each", [&]() { Query_empty_tables_each(); });
	It("Query_empty_tables_each_w_entity", [&]() { Query_empty_tables_each_w_entity(); });
	It("Query_empty_tables_each_w_iter", [&]() { Query_empty_tables_each_w_iter(); });
	It("Query_query_from_entity", [&]() { Query_query_from_entity(); });
	It("Query_query_from_entity_name", [&]() { Query_query_from_entity_name(); });
	It("Query_run_w_iter_fini", [&]() { Query_run_w_iter_fini(); });
	It("Query_run_w_iter_fini_interrupt", [&]() { Query_run_w_iter_fini_interrupt(); });
	It("Query_run_w_iter_fini_empty", [&]() { Query_run_w_iter_fini_empty(); });
	It("Query_run_w_iter_fini_no_query", [&]() { Query_run_w_iter_fini_no_query(); });
	It("Query_add_to_match_from_staged_query", [&]() { Query_add_to_match_from_staged_query(); });
	It("Query_add_to_match_from_staged_query_readonly_threaded", [&]() { Query_add_to_match_from_staged_query_readonly_threaded(); });
	It("Query_pair_with_variable_src", [&]() { Query_pair_with_variable_src(); });
	It("Query_pair_with_variable_src_no_row_fields", [&]() { Query_pair_with_variable_src_no_row_fields(); });
	It("Query_iter_targets", [&]() { Query_iter_targets(); });
	It("Query_iter_targets_2nd_field", [&]() { Query_iter_targets_2nd_field(); });
	It("Query_copy_operators", [&]() { Query_copy_operators(); });
}

#endif // WITH_AUTOMATION_TESTS
