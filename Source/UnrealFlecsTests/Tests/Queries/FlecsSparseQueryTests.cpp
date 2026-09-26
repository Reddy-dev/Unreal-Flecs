// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Properties/FlecsComponentProperties.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"

struct FFlecsSparseQueryTestPosition
{
	static constexpr bool dont_fragment = true;
	int32 Value = 0;
}; // struct FFlecsSparseQueryTestPosition

struct FFlecsSparseQueryTestVelocity
{
	static constexpr bool dont_fragment = true;
	int32 Value = 0;
}; // struct FFlecsSparseQueryTestVelocity

REGISTER_FLECS_COMPONENT(FFlecsSparseQueryTestPosition);
REGISTER_FLECS_COMPONENT(FFlecsSparseQueryTestVelocity);

static_assert(flecs::dont_fragment<FFlecsSparseQueryTestPosition>::value);
static_assert(flecs::dont_fragment<FFlecsSparseQueryTestVelocity>::value);
static_assert(TFlecsComponentTraits<FFlecsSparseQueryTestPosition>::DontFragment);
static_assert(TFlecsComponentTraits<FFlecsSparseQueryTestVelocity>::DontFragment);

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Queries/FlecsSparseQuery.h"
#include "Worlds/FlecsWorld.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS(FlecsSparseQueryTests,
								   "UnrealFlecs.Queries.Sparse",
							   EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
									| EAutomationTestFlags::CriticalPriority)
{
	TEST_METHOD(CreateSparseQuery_IteratesEntitiesWithEveryComponent_CPPAPI)
	{
		const FFlecsComponentHandle PositionComponent = World()->RegisterComponentType<FFlecsSparseQueryTestPosition>();
		const FFlecsComponentHandle VelocityComponent = World()->RegisterComponentType<FFlecsSparseQueryTestVelocity>();
		ASSERT_THAT(IsTrue(PositionComponent.Has(flecs::DontFragment)));
		ASSERT_THAT(IsTrue(VelocityComponent.Has(flecs::DontFragment)));

		World()->CreateEntity()
			.Set<FFlecsSparseQueryTestPosition>({ 10 })
			.Set<FFlecsSparseQueryTestVelocity>({ 100 });
		World()->CreateEntity()
			.Set<FFlecsSparseQueryTestPosition>({ 20 })
			.Set<FFlecsSparseQueryTestVelocity>({ 200 });
		World()->CreateEntity()
			.Set<FFlecsSparseQueryTestPosition>({ 30 });
		World()->CreateEntity()
			.Set<FFlecsSparseQueryTestVelocity>({ 300 });

		const TFlecsSparseQuery<const FFlecsSparseQueryTestPosition&, const FFlecsSparseQueryTestVelocity&> Query =
			World()->CreateSparseQuery<const FFlecsSparseQueryTestPosition&, const FFlecsSparseQueryTestVelocity&>();
		ASSERT_THAT(IsTrue(Query.count() == 2));

		int32 IteratedCount = 0;
		int32 PositionTotal = 0;
		int32 VelocityTotal = 0;
		Query.each([&](flecs::entity InEntity, const FFlecsSparseQueryTestPosition& InPosition,
			const FFlecsSparseQueryTestVelocity& InVelocity)
		{
			ASSERT_THAT(IsTrue(InEntity.is_alive()));
			PositionTotal += InPosition.Value;
			VelocityTotal += InVelocity.Value;
			++IteratedCount;
		});

		ASSERT_THAT(IsTrue(IteratedCount == 2));
		ASSERT_THAT(IsTrue(PositionTotal == 30));
		ASSERT_THAT(IsTrue(VelocityTotal == 300));
	}
}; // FlecsSparseQueryTests

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS
