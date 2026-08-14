#include <array>

#include "flecs.h"
#include "test.h"

namespace converter_fixture
{
	template <typename T>
	T Identity(T Value)
	{
		return Value;
	}

	static int Overloaded(int Value)
	{
		return Value;
	}

	static float Overloaded(float Value)
	{
		return Value;
	}

	// flecs-test: id=Fixture_templates_and_macros category=Fixture tags=template,macro
	void Fixture_templates_and_macros(void)
	{
		flecs::world World;
		const std::array<int, 2> Values{1, 2};
		test_assert(Identity(Values[0]) == Overloaded(1));
		test_bool(Values[1], 2);
	}
}

#if defined(FLECS_CONVERTER_FIXTURE)
	void Fixture_conditional(void)
	{
		test_assert(true);
	}
#endif

#define FLECS_CONVERTER_FIXTURE_TEST(Name) \
	void Name(void) \
	{ \
		test_assert(true); \
	}

// flecs-test: id=Fixture_macro_generated category=Fixture macro_generated=true
FLECS_CONVERTER_FIXTURE_TEST(Fixture_macro_generated)
