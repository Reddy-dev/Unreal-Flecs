#include "flecs.h"

// flecs-test: id=Fixture_assertions category=Fixture
void Fixture_assertions(void)
{
	flecs::world World;
	int Value = 1;
	test_assert(Value == 1);
	test_true(Value == 1);
	test_false(Value == 0);
	test_bool(true, Value == 1);
	test_int(Value, 1);
	test_uint(static_cast<unsigned int>(Value), 1u);
	test_flt(1.0f, 1.0f);
	test_str("fixture", "fixture");
	test_null(nullptr);
	test_not_null(&Value);
	test_ptr(&Value, &Value);
}
