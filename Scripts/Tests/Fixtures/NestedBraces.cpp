#include "flecs.h"
#include "test.h"

namespace converter_fixture
{
	struct Position
	{
		int X{};
	};

	static int GMutableState = 0;

	static void ResetMutableState(void)
	{
		GMutableState = 0;
	}

	// flecs-test: id=Fixture_nested_braces category=Fixture tags=ast,lambda reset_hook=ResetMutableState
	void Fixture_nested_braces(void)
	{
		flecs::world World;
		World.component<Position>();
		const auto Lambda = [&]()
		{
			if (GMutableState == 0)
			{
				GMutableState = 1;
			}
		};
		Lambda();
		test_assert(GMutableState == 1);
	}
}
