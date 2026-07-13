
#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "FlecsAbstractWorldSubsystemTestTypes.h"

#include "Networking/FlecsNetworkSubsystemSingleton.h"
#include "Networking/FlecsNetworkWorldSubsystem.h"

/*
 * Layout of Tests:
 * A. Abstract Flecs World Subsystem Initialization Tests
 */
TEST_CLASS_WITH_FLAGS_AND_TAGS(B3_FlecsWorldSubsystems, "UnrealFlecs.B3_FlecsWorldSubsystems",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
			| EAutomationTestFlags::CriticalPriority, "[Flecs]")
{
	inline static TUniquePtr<FFlecsTestFixtureRAII> Fixture;
	inline static TObjectPtr<UFlecsWorld> FlecsWorld = nullptr;

	BEFORE_EACH()
	{
		Fixture = MakeUnique<FFlecsTestFixtureRAII>();
		FlecsWorld = Fixture->Fixture.GetFlecsWorld();
	}
	
	AFTER_EACH()
	{
		FlecsWorld = nullptr;
		Fixture.Reset();
	}

	TEST_METHOD(A1_AbstractFlecsWorldSubsystem_FlecsWorldInitialization)
	{
		const UTestFlecsWorldSubsystem_Initialization* WorldSubsystem
			= FlecsWorld->GetWorld()->GetSubsystem<UTestFlecsWorldSubsystem_Initialization>();
		ASSERT_THAT(IsTrue(IsValid(WorldSubsystem)));
		
		ASSERT_THAT(IsTrue(WorldSubsystem->bWasFlecsWorldInitialized));
		ASSERT_THAT(IsTrue(IsValid(WorldSubsystem->GetFlecsWorld())));

		ASSERT_THAT(IsTrue(WorldSubsystem->TimesChecked == 0));
		++WorldSubsystem->TimesChecked;
	}
	
	TEST_METHOD(A2_AbstractFlecsWorldSubsystem_FlecsWorldInitialization_Again)
	{
		const UTestFlecsWorldSubsystem_Initialization* WorldSubsystem
			= FlecsWorld->GetWorld()->GetSubsystem<UTestFlecsWorldSubsystem_Initialization>();
		ASSERT_THAT(IsTrue(IsValid(WorldSubsystem)));
		
		ASSERT_THAT(IsTrue(WorldSubsystem->bWasFlecsWorldInitialized));
		ASSERT_THAT(IsTrue(IsValid(WorldSubsystem->GetFlecsWorld())));

		ASSERT_THAT(IsTrue(WorldSubsystem->TimesChecked == 0));
		++WorldSubsystem->TimesChecked;
	}

	TEST_METHOD(A4_NetworkSubsystemSingleton_IsAvailableWhenAbstractSubsystemsAreInitialized)
	{
		const UTestFlecsWorldSubsystem_Initialization* WorldSubsystem
			= FlecsWorld->GetWorld()->GetSubsystem<UTestFlecsWorldSubsystem_Initialization>();
		ASSERT_THAT(IsTrue(IsValid(WorldSubsystem)));

		ASSERT_THAT(IsTrue(WorldSubsystem->bWasFlecsWorldInitialized));
		ASSERT_THAT(IsFalse(WorldSubsystem->bWasNetworkSubsystemSingletonAvailable));
	}

	/*TEST_METHOD(A5_NetworkSubsystemSingleton_ReferencesOwningSubsystem)
	{
		const UFlecsNetworkWorldSubsystem* NetworkSubsystem
			= FlecsWorld->GetWorld()->GetSubsystem<UFlecsNetworkWorldSubsystem>();
		ASSERT_THAT(IsTrue(IsValid(NetworkSubsystem)));
		ASSERT_THAT(IsTrue(FlecsWorld->Has<FFlecsNetworkSubsystemSingleton>()));

		const FFlecsNetworkSubsystemSingleton& Singleton = FlecsWorld->Get<FFlecsNetworkSubsystemSingleton>();
		ASSERT_THAT(IsTrue(Singleton.IsValid()));
		ASSERT_THAT(IsTrue(Singleton.GetSubsystem<UFlecsNetworkWorldSubsystem>() == NetworkSubsystem));
	}*/
	
}; // End of B3_FlecsWorldSubsystems

#endif // WITH_AUTOMATION_TESTS
