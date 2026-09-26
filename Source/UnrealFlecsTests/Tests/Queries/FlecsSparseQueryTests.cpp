// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Queries/FlecsSparseQuery.h"
#include "Worlds/FlecsWorld.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS(FlecsSparseQueryTests,
								   "UnrealFlecs.Queries.Sparse",
							   EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
									| EAutomationTestFlags::CriticalPriority)
{
protected:
	virtual void OnRegisteredWorldSetUp() override
	{
		World()->RegisterComponentType<FFlecsSparseQueryTestPosition>();
		World()->RegisterComponentType<FFlecsSparseQueryTestVelocity>();
	}

public:
	TEST_METHOD(CreateSparseQuery_CountsEntitiesWithEveryComponent_CPPAPI)
	{
		const FFlecsComponentHandle PositionComponent = World()->RegisterComponentType<FFlecsSparseQueryTestPosition>();
		const FFlecsComponentHandle VelocityComponent = World()->RegisterComponentType<FFlecsSparseQueryTestVelocity>();
		ASSERT_THAT(IsTrue(PositionComponent.IsValid()));
		ASSERT_THAT(IsTrue(VelocityComponent.IsValid()));
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

		const TFlecsSparseQuery<FFlecsSparseQueryTestPosition, FFlecsSparseQueryTestVelocity> Query =
			World()->CreateSparseQuery<FFlecsSparseQueryTestPosition, FFlecsSparseQueryTestVelocity>();

		ASSERT_THAT(IsTrue(Query.count() == 2));
	}

	TEST_METHOD(CreateSparseQuery_EachWithEntityAndComponents_CPPAPI)
	{
		World()->CreateEntity()
			.Set<FFlecsSparseQueryTestPosition>({ 10 })
			.Set<FFlecsSparseQueryTestVelocity>({ 100 });
		World()->CreateEntity()
			.Set<FFlecsSparseQueryTestPosition>({ 20 })
			.Set<FFlecsSparseQueryTestVelocity>({ 200 });
		World()->CreateEntity()
			.Set<FFlecsSparseQueryTestPosition>({ 30 });

		const TFlecsSparseQuery<FFlecsSparseQueryTestPosition, FFlecsSparseQueryTestVelocity> Query =
			World()->CreateSparseQuery<FFlecsSparseQueryTestPosition, FFlecsSparseQueryTestVelocity>();
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

	TEST_METHOD(CreateSparseQuery_EachWithComponentValues_CPPAPI)
	{
		World()->CreateEntity().Set<FFlecsSparseQueryTestPosition>({ 10 });
		World()->CreateEntity().Set<FFlecsSparseQueryTestPosition>({ 20 });
		World()->CreateEntity().Set<FFlecsSparseQueryTestPosition>({ 30 });
		World()->CreateEntity().Set<FFlecsSparseQueryTestVelocity>({ 100 });

		const TFlecsSparseQuery<FFlecsSparseQueryTestPosition> Query =
			World()->CreateSparseQuery<FFlecsSparseQueryTestPosition>();
		int32 IteratedCount = 0;
		int32 PositionTotal = 0;

		Query.each([&](const FFlecsSparseQueryTestPosition& InPosition)
		{
			PositionTotal += InPosition.Value;
			++IteratedCount;
		});

		ASSERT_THAT(IsTrue(IteratedCount == 3));
		ASSERT_THAT(IsTrue(PositionTotal == 60));
	}
}; // FlecsSparseQueryTests

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS
