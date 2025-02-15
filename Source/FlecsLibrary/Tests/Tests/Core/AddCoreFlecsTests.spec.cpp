#if WITH_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "FlecsLibrary/Tests/Bake/FlecsTestUtils.h"
#include "FlecsCoreTestTypes.h"
#include <stdlib.h>

BEGIN_DEFINE_SPEC(FAddFlecsTestsSpec,
                  "FlecsLibrary.Core.Add",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

void Add_component(void) {
    ecs_world_t *world = ecs_mini();

    ECS_COMPONENT(world, Position);

    ecs_entity_t e = ecs_new(world);
    test_assert(e != 0);

    ecs_add(world, e, Position);
    test_assert(ecs_has(world, e, Position));
    
    ecs_fini(world);
}

void Add_component_again(void) {
    ecs_world_t *world = ecs_mini();

    ECS_COMPONENT(world, Position);

    ecs_entity_t e = ecs_new(world);
    test_assert(e != 0);

    ecs_add(world, e, Position);
    test_assert(ecs_has(world, e, Position));

    ecs_add(world, e, Position);
    test_assert(ecs_has(world, e, Position));
    
    ecs_fini(world);
}

void Add_2_components(void) {
    ecs_world_t *world = ecs_mini();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);

    ecs_entity_t e = ecs_new(world);
    test_assert(e != 0);

    ecs_add(world, e, Position);
    test_assert(ecs_has(world, e, Position));
    test_assert(!ecs_has(world, e, Velocity));

    ecs_add(world, e, Velocity);
    test_assert(ecs_has(world, e, Position));
    test_assert(ecs_has(world, e, Velocity));
    
    ecs_fini(world);
}

void Add_2_components_again(void) {
    ecs_world_t *world = ecs_mini();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);

    ecs_entity_t e = ecs_new(world);
    test_assert(e != 0);

    ecs_add(world, e, Position);
    ecs_add(world, e, Velocity);
    test_assert(ecs_has(world, e, Position));
    test_assert(ecs_has(world, e, Velocity));

    ecs_add(world, e, Position);
    ecs_add(world, e, Velocity);
    test_assert(ecs_has(world, e, Position));
    test_assert(ecs_has(world, e, Velocity));
    
    ecs_fini(world);
}

void Add_2_components_overlap(void) {
    ecs_world_t *world = ecs_mini();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_COMPONENT(world, Mass);

    ecs_entity_t e = ecs_new(world);
    test_assert(e != 0);

    ecs_add(world, e, Position);
    ecs_add(world, e, Velocity);
    test_assert(ecs_has(world, e, Position));
    test_assert(ecs_has(world, e, Velocity));
    test_assert(!ecs_has(world, e, Mass));

    ecs_add(world, e, Velocity);
    ecs_add(world, e, Mass);
    test_assert(ecs_has(world, e, Position));
    test_assert(ecs_has(world, e, Velocity));
    test_assert(ecs_has(world, e, Mass));
    
    ecs_fini(world);
}

void Add_component_to_nonempty(void) {
    ecs_world_t *world = ecs_mini();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);

    ecs_entity_t e = ecs_new_w(world, Position);
    test_assert(e != 0);
    test_assert(ecs_has(world, e, Position));

    ecs_add(world, e, Velocity);
    test_assert(ecs_has(world, e, Position));
    test_assert(ecs_has(world, e, Velocity));

    ecs_fini(world);
}

void Add_component_to_nonempty_again(void) {
    ecs_world_t *world = ecs_mini();

    ECS_COMPONENT(world, Position);

    ecs_entity_t e = ecs_new_w(world, Position);
    test_assert(e != 0);
    test_assert(ecs_has(world, e, Position));

    ecs_add(world, e, Position);
    test_assert(ecs_has(world, e, Position));

    ecs_fini(world);
}

void Add_component_to_nonempty_overlap(void) {
    ecs_world_t *world = ecs_mini();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);

    ECS_ENTITY(world, e, Position, Velocity);
    test_assert(e != 0);
    test_assert(ecs_has(world, e, Position));
    test_assert(ecs_has(world, e, Velocity));

    ecs_add(world, e, Position);

    test_assert(ecs_has(world, e, Position));
    test_assert(ecs_has(world, e, Velocity));

    ecs_fini(world);
}

void Add_tag(void) {
    ecs_world_t *world = ecs_mini();

    ECS_ENTITY(world, Tag, 0);

    ecs_entity_t e = ecs_new(world);
    test_assert(e != 0);

    ecs_add_id(world, e, Tag);
    test_assert(ecs_has_id(world, e, Tag));

    ecs_fini(world);
}

void Add_add_entity(void) {
    ecs_world_t *world = ecs_mini();

    ecs_entity_t e = ecs_new(world);
    test_assert(e != 0);

    ecs_entity_t f = ecs_new(world);
    test_assert(f != 0);

    ecs_add_id(world, e, f);
    test_assert(ecs_has_id(world, e, f));
    
    ecs_fini(world);
}

void Add_remove_entity(void) {
    ecs_world_t *world = ecs_mini();

    ecs_entity_t e = ecs_new(world);
    test_assert(e != 0);

    ecs_entity_t f = ecs_new(world);
    test_assert(f != 0);

    ecs_add_id(world, e, f);
    test_assert(ecs_has_id(world, e, f));

    ecs_remove_id(world, e, f);
    test_assert(!ecs_has_id(world, e, f));
    
    ecs_fini(world);
}

void Add_add_random_id(void) {
    ecs_world_t *world = ecs_mini();

    for (int i = 0; i < 10; i ++) {
        ecs_entity_t id = rand() % (100 * 1000) + 1000;
        ecs_make_alive(world, id);
        test_assert(ecs_is_alive(world, id));
        ecs_entity_t e = ecs_new_w_id(world, id);
        test_assert(ecs_has_id(world, e, id));
    }

    ecs_fini(world);
}

END_DEFINE_SPEC(FAddFlecsTestsSpec);

void FAddFlecsTestsSpec::Define()
{
	Describe("Components", [this]()
	{
	    It ("Should add a component", [this]()
	    {
	        Add_component();
	    });

	    It ("Should add a component again", [this]()
        {
            Add_component_again();
        });

	    It ("Should add 2 components", [this]()
        {
            Add_2_components();
        });

	    It ("Should add 2 components again", [this]()
        {
            Add_2_components_again();
        });

	    It("Should add 2 components with overlap", [this]()
        {
            Add_2_components_overlap();
        });

	    It("Should add component to nonempty entity", [this]()
        {
            Add_component_to_nonempty();
        });

	    It("Should add component to nonempty entity again", [this]()
        {
            Add_component_to_nonempty_again();
        });

	    It("Should add component to nonempty entity with overlap", [this]()
        {
            Add_component_to_nonempty_overlap();
        });
	});

    Describe("Tags/Ids", [this]()
    {
        It("Should add a tag", [this]()
        {
            Add_tag();
        });

        It("Should add an entity", [this]()
        {
            Add_add_entity();
        });

        It("Should remove an entity", [this]()
        {
            Add_remove_entity();
        });

        It("Should add random id", [this]()
        {
            Add_add_random_id();
        });

        It("Should remove random id", [this]()
        {
            Add_remove_entity();
        });
    });
}

#endif // WITH_AUTOMATION_TESTS
