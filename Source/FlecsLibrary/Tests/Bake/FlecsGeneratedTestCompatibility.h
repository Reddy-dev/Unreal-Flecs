// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "Bake/FlecsGeneratedTestUtils.h"
#include "Bake/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS

// The upstream Bake harness exposes this spelling. Keep it in the generated
// test compatibility layer instead of adding converter-only names to the
// shared test-type header used by hand-authored tests.
using LifecycleTracker = FlecsTestLifecycleTracker;

namespace FlecsGeneratedTest
{
	inline void RegisterSharedTypes(flecs::world& World)
	{
		RegisterTestTypeComponents(World);
	}
}

#endif // WITH_AUTOMATION_TESTS
