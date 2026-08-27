// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsTestCase.h"
#include "UnrealFlecsTests/Fixtures/FlecsWorldFixture.h"

#include "UnrealFlecsConfigMacros.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsFixtureFrameworkTests,
	"UnrealFlecs.TestFramework.Fixtures",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter,
	"[Flecs][TestFramework]")
{
	TEST_METHOD(WorldFixture_CreatesAValidWorldForEachMethod)
	{
		ASSERT_THAT(IsNotNull(World()));
		ASSERT_THAT(IsNotNull(WorldSubsystem()));
		ASSERT_THAT(IsNotNull(UnrealWorld()));
		ASSERT_THAT(IsTrue(World()->CreateEntity("FixtureState").IsValid()));
	}

	TEST_METHOD(WorldFixture_PreviousMethodStateDoesNotLeak)
	{
		ASSERT_THAT(IsFalse(World()->LookupEntity("FixtureState").IsValid()));
		ASSERT_THAT(IsTrue(World()->CreateEntity("FixtureState").IsValid()));
	}

	TEST_METHOD(WorldFixture_TearDownIsIdempotent)
	{
		FFlecsTestFixture LocalFixture;
		LocalFixture.SetUp();
		ASSERT_THAT(IsNotNull(LocalFixture.GetFlecsWorld()));

		LocalFixture.TearDown();
		LocalFixture.TearDown();

		ASSERT_THAT(IsNull(LocalFixture.GetFlecsWorld()));
		ASSERT_THAT(IsNull(LocalFixture.GetWorldSubsystem()));
		ASSERT_THAT(IsNull(LocalFixture.GetTestWorld()));
	}

	TEST_METHOD(NamedCaseTable_ExecutesEveryRowWithAssertionContext)
	{
		const TArray<FFlecsNamedTestCase<int32>> Cases =
		{
			{ TEXT("FirstRow"), 2 },
			{ TEXT("SecondRow"), 3 },
			{ TEXT("ThirdRow"), 5 },
		};

		int32 ExecutedRows = 0;
		int32 Total = 0;
		ForEachFlecsTestCase(*this, MakeArrayView(Cases),
			[this, &ExecutedRows, &Total](const FFlecsNamedTestCase<int32>& TestCase)
			{
				FAutomationTestExecutionInfo ExecutionInfo;
				TestRunner->GetExecutionInfo(ExecutionInfo);
				ASSERT_THAT(AreEqual(TestCase.Name, ExecutionInfo.GetContext()));

				++ExecutedRows;
				Total += TestCase.Value;
			});

		ASSERT_THAT(AreEqual(3, ExecutedRows));
		ASSERT_THAT(AreEqual(10, Total));
	}
}; // FlecsFixtureFrameworkTests

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS
