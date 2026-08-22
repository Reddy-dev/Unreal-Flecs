// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsWorldFixture.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Pipelines/FlecsOutsideMainLoopTag.h"
#include "Systems/FlecsPhasesType.h"
#include "Systems/FlecsSystemHandle.h"
#include "UObject/UObjectGlobals.h"
#include "Worlds/FlecsWorld.h"
#include "UnrealFlecsTests/Tests/Types/FlecsSystemObjectTestTypes.h"

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(UnrealFlecsSystemRunTests,
	"UnrealFlecs.Pipelines.SystemRun",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
	| EAutomationTestFlags::CriticalPriority,
	"[Flecs][Pipelines][Systems]")
{
protected:
	virtual EWorldType::Type WorldType() const override
	{
		return EWorldType::Game;
	}

public:
	TEST_METHOD(RunSystemManually)
	{
		static constexpr double PipelineDeltaTime = 1.0 / 60.0;
		static constexpr double ManualDeltaTime = 1.0 / 30.0;

		int32 RunCount = 0;
		double LastDeltaTime = 0.0;
		const FFlecsSystemHandle System = World()->CreateSystem<>(TEXT("ManualSystem"))
			.Phase(EFlecsPhaseType::OnUpdate)
			.run([&RunCount, &LastDeltaTime](flecs::iter& InIterator)
			{
				while (InIterator.next())
				{
					++RunCount;
					LastDeltaTime = InIterator.delta_time();
				}
			});

		ASSERT_THAT(IsTrue(System.IsValid()));
		ASSERT_THAT(AreEqual(0, RunCount));

		TickWorld(PipelineDeltaTime);

		ASSERT_THAT(AreEqual(1, RunCount));
		ASSERT_THAT(IsTrue(FMath::IsNearlyEqual(LastDeltaTime, PipelineDeltaTime)));

		System.Run(ManualDeltaTime);

		ASSERT_THAT(AreEqual(2, RunCount));
		ASSERT_THAT(IsTrue(FMath::IsNearlyEqual(LastDeltaTime, ManualDeltaTime)));
	}

	TEST_METHOD(SystemOutsideMainLoop_RunsOnlyWhenRunManually)
	{
		static constexpr double ManualDeltaTime = 1.0 / 30.0;

		int32 RunCount = 0;
		const FFlecsSystemHandle System = World()->CreateSystem<>(TEXT("ManualOnlySystem"))
			.Phase(EFlecsPhaseType::OnUpdate)
			.run([&RunCount](flecs::iter& InIterator)
			{
				while (InIterator.next())
				{
					++RunCount;
				}
			});
		System.Add<FFlecsOutsideMainLoopTag>();

		ASSERT_THAT(IsTrue(System.IsValid()));
		ASSERT_THAT(IsTrue(System.Has<FFlecsOutsideMainLoopTag>()));

		TickWorld();

		ASSERT_THAT(AreEqual(0, RunCount));

		System.Run(ManualDeltaTime);

		ASSERT_THAT(AreEqual(1, RunCount));

		TickWorld();

		ASSERT_THAT(AreEqual(1, RunCount));
	}

	TEST_METHOD(DisabledSystem_RunsOnlyWhenRunManually)
	{
		static constexpr double ManualDeltaTime = 1.0 / 30.0;

		int32 RunCount = 0;
		const FFlecsSystemHandle System = World()->CreateSystem<>(TEXT("DisabledSystem"))
			.Phase(EFlecsPhaseType::OnUpdate)
			.run([&RunCount](flecs::iter& InIterator)
			{
				while (InIterator.next())
				{
					++RunCount;
				}
			});
		System.Add(flecs::Disabled);

		ASSERT_THAT(IsTrue(System.IsValid()));
		ASSERT_THAT(IsTrue(System.Has(flecs::Disabled)));

		TickWorld();

		ASSERT_THAT(AreEqual(0, RunCount));

		System.Run(ManualDeltaTime);

		ASSERT_THAT(AreEqual(1, RunCount));

		TickWorld();

		ASSERT_THAT(AreEqual(1, RunCount));
	}

	TEST_METHOD(ObjectStartsDisabled_DisablesRegisteredSystemUntilManuallyRun)
	{
		static constexpr double ManualDeltaTime = 1.0 / 30.0;

		UFlecsStartsDisabledSystemTestObject* SystemCDO =
			GetMutableDefault<UFlecsStartsDisabledSystemTestObject>();
		const bool bPreviousStartsDisabled = SystemCDO->GetStartsDisabled();
		SystemCDO->SetStartsDisabled(true);

		UFlecsStartsDisabledSystemTestObject* SystemObject =
			World()->RegisterFlecsObject<UFlecsStartsDisabledSystemTestObject>();

		SystemCDO->SetStartsDisabled(bPreviousStartsDisabled);

		ASSERT_THAT(IsNotNull(SystemObject));
		ASSERT_THAT(IsTrue(SystemObject->GetSystemHandle().IsValid()));
		ASSERT_THAT(IsTrue(SystemObject->GetSystemHandle().Has(flecs::Disabled)));
		ASSERT_THAT(AreEqual(0, SystemObject->GetRunCount()));

		TickWorld();

		ASSERT_THAT(AreEqual(0, SystemObject->GetRunCount()));

		SystemObject->RunSystem(ManualDeltaTime);

		ASSERT_THAT(AreEqual(1, SystemObject->GetRunCount()));
		ASSERT_THAT(IsTrue(SystemObject->GetSystemHandle().Has(flecs::Disabled)));

		TickWorld();

		ASSERT_THAT(AreEqual(1, SystemObject->GetRunCount()));
	}
}; // UnrealFlecsSystemRunTests

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS
