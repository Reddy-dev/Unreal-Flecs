#if WITH_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "FlecsLibrary/Tests/Bake/FlecsTestUtils.h"
#include "FlecsCoreTestTypes.h"

MSVC_WARNING_PUSH
MSVC_WARNING_DISABLE(4576) // a parenthesized type followed by an initializer list is a non-standard explicit type conversion syntax

static int ctor_flecs_test_Position = 0;
static
ECS_CTOR(flecs_test_Position, ptr, {
    ptr->x = 7;
    ptr->y = 9;
    ctor_flecs_test_Position ++;
})

static int dtor_flecs_test_Position = 0;
static
ECS_DTOR(flecs_test_Position, ptr, {
    dtor_flecs_test_Position ++;
})

static int copy_flecs_test_Position = 0;
static
ECS_COPY(flecs_test_Position, dst, src, {
    copy_flecs_test_Position ++;
    *dst = *src;
})

static int move_flecs_test_Position = 0;
static
ECS_MOVE(flecs_test_Position, dst, src, {
    move_flecs_test_Position ++;
    *dst = *src;
})

BEGIN_DEFINE_SPEC(FCloneCoreFlecsTestsSpec,
                  "FlecsLibrary.Core.Clone",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

void Clone_empty(void) {
    ecs_world_t *world = ecs_mini();

    ecs_entity_t e1 = ecs_new(world);
    test_assert(e1 != 0);

    ecs_entity_t e2 = ecs_clone(world, 0, e1, false);
    test_assert(e2 != 0);
    test_assert(e1 != e2);

    test_assert(!ecs_get_type(world, e1));
    test_assert(!ecs_get_type(world, e2));

    ecs_fini(world);
}

void Clone_empty_w_value(void) {
    ecs_world_t *world = ecs_mini();

    ecs_entity_t e1 = ecs_new(world);
    test_assert(e1 != 0);

    ecs_entity_t e2 = ecs_clone(world, 0, e1, true);
    test_assert(e2 != 0);
    test_assert(e1 != e2);

    test_assert(!ecs_get_type(world, e1));
    test_assert(!ecs_get_type(world, e2));

    ecs_fini(world);
}

void Clone_1_component(void) {
    ecs_world_t *world = ecs_mini();

    ECS_COMPONENT(world, flecs_test_Position);

    ecs_entity_t e1 = ecs_new_w(world, flecs_test_Position);
    test_assert(e1 != 0);

    ecs_entity_t e2 = ecs_clone(world, 0, e1, false);
    test_assert(e2 != 0);
    test_assert(e1 != e2);

    test_assert(ecs_has(world, e1, flecs_test_Position));
    test_assert(ecs_has(world, e2, flecs_test_Position));
    test_assert(ecs_get(world, e1, flecs_test_Position) !=  
                ecs_get(world, e2, flecs_test_Position));

    ecs_fini(world);
}

void Clone_2_component(void) {
    ecs_world_t *world = ecs_mini();

    ECS_COMPONENT(world, flecs_test_Position);
    ECS_COMPONENT(world, flecs_test_Velocity);

    ECS_ENTITY(world, e1, flecs_test_Position, flecs_test_Velocity);
    test_assert(e1 != 0);

    ecs_entity_t e2 = ecs_clone(world, 0, e1, false);
    test_assert(e2 != 0);
    test_assert(e1 != e2);

    test_assert(ecs_has(world, e1, flecs_test_Position));
    test_assert(ecs_has(world, e2, flecs_test_Position));
    test_assert(ecs_get(world, e1, flecs_test_Position) !=  
                ecs_get(world, e2, flecs_test_Position));

    test_assert(ecs_has(world, e1, flecs_test_Velocity));
    test_assert(ecs_has(world, e2, flecs_test_Velocity));
    test_assert(ecs_get(world, e1, flecs_test_Velocity) !=  
                ecs_get(world, e2, flecs_test_Velocity));

    ecs_fini(world);
}

void Clone_3_component(void) {
    ecs_world_t *world = ecs_mini();

    ECS_COMPONENT(world, flecs_test_Position);
    ECS_COMPONENT(world, flecs_test_Velocity);
    ECS_COMPONENT(world, flecs_test_Mass);

    ECS_ENTITY(world, e1, flecs_test_Position, flecs_test_Velocity, flecs_test_Mass);
    test_assert(e1 != 0);

    ecs_entity_t e2 = ecs_clone(world, 0, e1, false);
    test_assert(e2 != 0);
    test_assert(e1 != e2);

    test_assert(ecs_has(world, e1, flecs_test_Position));
    test_assert(ecs_has(world, e2, flecs_test_Position));
    test_assert(ecs_get(world, e1, flecs_test_Position) !=  
                ecs_get(world, e2, flecs_test_Position));

    test_assert(ecs_has(world, e1, flecs_test_Velocity));
    test_assert(ecs_has(world, e2, flecs_test_Velocity));
    test_assert(ecs_get(world, e1, flecs_test_Velocity) !=  
                ecs_get(world, e2, flecs_test_Velocity));

    test_assert(ecs_has(world, e1, flecs_test_Mass));
    test_assert(ecs_has(world, e2, flecs_test_Mass));
    test_assert(ecs_get(world, e1, flecs_test_Mass) !=  
                ecs_get(world, e2, flecs_test_Mass));

    ecs_fini(world);
}

void Clone_1_component_w_value(void) {
    ecs_world_t *world = ecs_mini();

    ECS_COMPONENT(world, flecs_test_Position);

    ecs_entity_t e1 = ecs_new(world);
    test_assert(e1 != 0);

    ecs_set(world, e1, flecs_test_Position, {10, 20});

    ecs_entity_t e2 = ecs_clone(world, 0, e1, true);
    test_assert(e2 != 0);
    test_assert(e1 != e2);

    test_assert(ecs_has(world, e1, flecs_test_Position));
    test_assert(ecs_has(world, e2, flecs_test_Position));

    const flecs_test_Position *p_1 = ecs_get(world, e1, flecs_test_Position);
    test_assert(p_1 != NULL);
    test_int(p_1->x, 10);
    test_int(p_1->y, 20);

    const flecs_test_Position *p_2 = ecs_get(world, e2, flecs_test_Position);
    test_assert(p_2 != NULL);
    test_assert(p_1 != p_2);
    test_int(p_2->x, 10);
    test_int(p_2->y, 20);

    ecs_fini(world);
}

void Clone_2_component_w_value(void) {
    ecs_world_t *world = ecs_mini();

    ECS_COMPONENT(world, flecs_test_Position);
    ECS_COMPONENT(world, flecs_test_Velocity);

    ecs_entity_t e1 = ecs_new(world);
    test_assert(e1 != 0);

    ecs_set(world, e1, flecs_test_Position, {10, 20});
    ecs_set(world, e1, flecs_test_Velocity, {30, 40});

    ecs_entity_t e2 = ecs_clone(world, 0, e1, true);
    test_assert(e2 != 0);
    test_assert(e1 != e2);

    test_assert(ecs_has(world, e1, flecs_test_Position));
    test_assert(ecs_has(world, e2, flecs_test_Position));
    test_assert(ecs_has(world, e1, flecs_test_Velocity));
    test_assert(ecs_has(world, e2, flecs_test_Velocity));

    const flecs_test_Position *p_1 = ecs_get(world, e1, flecs_test_Position);
    test_assert(p_1 != NULL);
    test_int(p_1->x, 10);
    test_int(p_1->y, 20);

    const flecs_test_Position *p_2 = ecs_get(world, e2, flecs_test_Position);
    test_assert(p_2 != NULL);
    test_assert(p_1 != p_2);
    test_int(p_2->x, 10);
    test_int(p_2->y, 20);

    const flecs_test_Velocity *v_1 = ecs_get(world, e1, flecs_test_Velocity);
    test_assert(v_1 != NULL);
    test_int(v_1->x, 30);
    test_int(v_1->y, 40);

    const flecs_test_Velocity *v_2 = ecs_get(world, e2, flecs_test_Velocity);
    test_assert(v_2 != NULL);
    test_assert(v_1 != v_2);
    test_int(v_2->x, 30);
    test_int(v_2->y, 40);

    ecs_fini(world);
}

void Clone_3_component_w_value(void) {
    ecs_world_t *world = ecs_mini();

    ECS_COMPONENT(world, flecs_test_Position);
    ECS_COMPONENT(world, flecs_test_Velocity);
    ECS_COMPONENT(world, flecs_test_Mass);

    ecs_entity_t e1 = ecs_new(world);
    test_assert(e1 != 0);

    ecs_set(world, e1, flecs_test_Position, {10.f, 20.f});
    ecs_set(world, e1, flecs_test_Velocity, {30.f, 40.f});
    ecs_set(world, e1, flecs_test_Mass, { 50 });

    ecs_entity_t e2 = ecs_clone(world, 0, e1, true);
    test_assert(e2 != 0);
    test_assert(e1 != e2);

    test_assert(ecs_has(world, e1, flecs_test_Position));
    test_assert(ecs_has(world, e2, flecs_test_Position));
    test_assert(ecs_has(world, e1, flecs_test_Velocity));
    test_assert(ecs_has(world, e2, flecs_test_Velocity));
    test_assert(ecs_has(world, e1, flecs_test_Mass));
    test_assert(ecs_has(world, e2, flecs_test_Mass));

    const flecs_test_Position *p_1 = ecs_get(world, e1, flecs_test_Position);
    test_assert(p_1 != NULL);
    test_int(p_1->x, 10);
    test_int(p_1->y, 20);

    const flecs_test_Position *p_2 = ecs_get(world, e2, flecs_test_Position);
    test_assert(p_2 != NULL);
    test_assert(p_1 != p_2);
    test_int(p_2->x, 10);
    test_int(p_2->y, 20);

    const flecs_test_Velocity *v_1 = ecs_get(world, e1, flecs_test_Velocity);
    test_assert(v_1 != NULL);
    test_int(v_1->x, 30);
    test_int(v_1->y, 40);

    const flecs_test_Velocity *v_2 = ecs_get(world, e2, flecs_test_Velocity);
    test_assert(v_2 != NULL);
    test_assert(v_1 != v_2);
    test_int(v_2->x, 30);
    test_int(v_2->y, 40);

    const flecs_test_Mass *m_1 = ecs_get(world, e1, flecs_test_Mass);
    test_assert(m_1 != NULL);
    test_int(*m_1, 50);

    const flecs_test_Mass *m_2 = ecs_get(world, e2, flecs_test_Mass);
    test_assert(m_2 != NULL);
    test_assert(m_1 != m_2);
    test_int(*m_2, 50);

    ecs_fini(world);
}

void Clone_1_component_w_lifecycle(void) {
    ecs_world_t *world = ecs_mini();

    ECS_COMPONENT(world, flecs_test_Position);
        ecs_set_hooks(world, flecs_test_Position, {
        .ctor = ecs_ctor(flecs_test_Position),
        .dtor = ecs_dtor(flecs_test_Position),
        .copy = ecs_copy(flecs_test_Position),
        .move = ecs_move(flecs_test_Position)
    });

    ecs_entity_t e1 = ecs_new(world);
    test_assert(e1 != 0);

    flecs_test_Position * p = ecs_ensure(world, e1, flecs_test_Position);
    test_int(1, ctor_flecs_test_Position);
    /* Check we get the special default values as per the ctor above */
    test_int(7, p->x);
    test_int(9, p->y);

    /* Change the values to something different than default */
    p->x = 1;
    p->y = 2;

    ecs_entity_t e2 = ecs_clone(world, 0, e1, true);
    /* Test for leaks. Only 2 flecs_test_Position objects should be alive */
    test_int(2, ctor_flecs_test_Position - dtor_flecs_test_Position);
    test_assert(e2 != 0);
    test_assert(e1 != e2);

    test_assert(ecs_has(world, e1, flecs_test_Position));
    test_assert(ecs_has(world, e2, flecs_test_Position));

    const flecs_test_Position *p_1 = ecs_get(world, e1, flecs_test_Position);
    test_assert(p_1 != NULL);
    test_int(1, p_1->x);
    test_int(2, p_1->y);

    const flecs_test_Position *p_2 = ecs_get(world, e2, flecs_test_Position);
    test_assert(p_2 != NULL);
    test_assert(p_1 != p_2);
    test_int(1, p_2->x);
    test_int(2, p_2->y);

    ecs_fini(world);
    test_int(0, ctor_flecs_test_Position - dtor_flecs_test_Position); /* test for leaks */
}

void Clone_tag(void) {
    ecs_world_t *world = ecs_mini();

    ECS_ENTITY(world, Tag, 0);

    ecs_entity_t e1 = ecs_new_w_id(world, Tag);
    test_assert(e1 != 0);

    ecs_entity_t e2 = ecs_clone(world, 0, e1, false);
    test_assert(e2 != 0);
    test_assert(e1 != e2);

    test_assert(ecs_has_id(world, e1, Tag));
    test_assert(ecs_has_id(world, e2, Tag));

    ecs_fini(world);
}

void Clone_tag_w_value(void) {
    ecs_world_t *world = ecs_mini();

    ECS_ENTITY(world, Tag, 0);

    ecs_entity_t e1 = ecs_new_w_id(world, Tag);
    test_assert(e1 != 0);

    ecs_entity_t e2 = ecs_clone(world, 0, e1, true);
    test_assert(e2 != 0);
    test_assert(e1 != e2);

    test_assert(ecs_has_id(world, e1, Tag));
    test_assert(ecs_has_id(world, e2, Tag));

    ecs_fini(world);
}

void Clone_1_tag_1_component(void) {
    ecs_world_t *world = ecs_mini();

    ECS_ENTITY(world, Tag, 0);
    ECS_COMPONENT(world, flecs_test_Position);
    ECS_ENTITY(world, e1, flecs_test_Position, Tag);

    ecs_set(world, e1, flecs_test_Position, {10, 20});

    ecs_entity_t e2 = ecs_clone(world, 0, e1, false);
    test_assert(e2 != 0);
    test_assert(e1 != e2);

    test_assert(ecs_has_id(world, e1, Tag));
    test_assert(ecs_has_id(world, e2, Tag));

    test_assert(ecs_has(world, e1, flecs_test_Position));
    test_assert(ecs_has(world, e2, flecs_test_Position));

    ecs_fini(world);
}

void Clone_1_tag_1_component_w_value(void) {
    ecs_world_t *world = ecs_mini();

    ECS_ENTITY(world, Tag, 0);
    ECS_COMPONENT(world, flecs_test_Position);

    ecs_entity_t e1 = ecs_new(world);
    ecs_add(world, e1, Tag);
    ecs_set(world, e1, flecs_test_Position, {10, 20});

    ecs_entity_t e2 = ecs_clone(world, 0, e1, true);
    test_assert(e2 != 0);
    test_assert(e1 != e2);

    test_assert(ecs_has_id(world, e1, Tag));
    test_assert(ecs_has_id(world, e2, Tag));

    test_assert(ecs_has(world, e1, flecs_test_Position));
    test_assert(ecs_has(world, e2, flecs_test_Position));

    const flecs_test_Position *p = ecs_get(world, e2, flecs_test_Position);
    test_assert(p != NULL);
    test_int(p->x, 10);
    test_int(p->y, 20);

    ecs_fini(world);
}

void Clone_clone_w_name(void) {
    ecs_world_t *world = ecs_mini();

    ECS_COMPONENT(world, flecs_test_Position);
    ECS_COMPONENT(world, flecs_test_Velocity);

    ecs_entity_t t = ecs_entity(world, { .name = "template" });
    ecs_set(world, t, flecs_test_Position, {10, 20});
    ecs_set(world, t, flecs_test_Velocity, {1, 2});

    ecs_entity_t i = ecs_clone(world, 0, t, true);
    test_assert(ecs_has(world, i, flecs_test_Position));
    test_assert(ecs_has(world, i, flecs_test_Velocity));
    test_assert(!ecs_has_pair(world, i, ecs_id(EcsIdentifier), EcsName));
    test_assert(ecs_lookup(world, "template") == t);

    const flecs_test_Position *p = ecs_get(world, i, flecs_test_Position);
    test_assert(p != NULL);
    test_int(p->x, 10);
    test_int(p->y, 20);

    const flecs_test_Velocity *v = ecs_get(world, i, flecs_test_Velocity);
    test_assert(v != NULL);
    test_int(v->x, 1);
    test_int(v->y, 2); 

    ecs_fini(world);
}

END_DEFINE_SPEC(FCloneCoreFlecsTestsSpec);

void FCloneCoreFlecsTestsSpec::Define()
{
	Describe("Clone Empty", [this]()
	{
	    It("Should clone empty entity", [this]()
        {
            Clone_empty();
        });

        It("Should clone empty entity with value", [this]()
        {
            Clone_empty_w_value();
        });
    });

    Describe("Clone Components", [this]()
    {
        It("Should clone 1 component", [this]()
        {
            Clone_1_component();
        });

        It("Should clone 2 components", [this]()
        {
            Clone_2_component();
        });

        It("Should clone 3 components", [this]()
        {
            Clone_3_component();
        });

        It("Should clone 1 component with value", [this]()
        {
            Clone_1_component_w_value();
        });

        It("Should clone 2 components with value", [this]()
        {
            Clone_2_component_w_value();
        });

        It("Should clone 3 components with value", [this]()
        {
            Clone_3_component_w_value();
        });

        It("Should clone 1 component with lifecycle callbacks", [this]()
        {
            Clone_1_component_w_lifecycle();
        });
    });

    Describe("Clone Tags", [this]()
    {
        It("Should clone tag", [this]()
        {
            Clone_tag();
        });

        It("Should clone tag with value", [this]()
        {
            Clone_tag_w_value();
        });

        It("Should clone tag and component", [this]()
        {
            Clone_1_tag_1_component();
        });

        It("Should clone tag and component with value", [this]()
        {
            Clone_1_tag_1_component_w_value();
        });
    });

    Describe("Clone Name", [this]()
    {
        It("Should clone entity with name", [this]()
        {
            Clone_clone_w_name();
        });
    });
}

MSVC_WARNING_POP

#endif // WITH_AUTOMATION_TESTS
