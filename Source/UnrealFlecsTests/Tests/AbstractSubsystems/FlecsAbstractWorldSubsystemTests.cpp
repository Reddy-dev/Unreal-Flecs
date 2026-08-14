
#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "FlecsAbstractWorldSubsystemTestTypes.h"

#include "Networking/Subsystem/FlecsNetworkSubsystemSingleton.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsWorldSubsystems, "UnrealFlecs.World.Subsystems",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
			| EAutomationTestFlags::CriticalPriority, "[Flecs]")
{
	TEST_METHOD(AbstractFlecsWorldSubsystem_FlecsWorldInitialization)
	{
		const UTestFlecsWorldSubsystem_Initialization* WorldSubsystem
			= World()->GetWorld()->GetSubsystem<UTestFlecsWorldSubsystem_Initialization>();
		ASSERT_THAT(IsTrue(IsValid(WorldSubsystem)));
		
		ASSERT_THAT(IsTrue(WorldSubsystem->bWasFlecsWorldInitialized));
		ASSERT_THAT(IsTrue(IsValid(WorldSubsystem->GetFlecsWorld())));

		ASSERT_THAT(IsTrue(WorldSubsystem->TimesChecked == 0));
		++WorldSubsystem->TimesChecked;
	}
	
	TEST_METHOD(AbstractFlecsWorldSubsystem_FlecsWorldInitialization_Again)
	{
		const UTestFlecsWorldSubsystem_Initialization* WorldSubsystem
			= World()->GetWorld()->GetSubsystem<UTestFlecsWorldSubsystem_Initialization>();
		ASSERT_THAT(IsTrue(IsValid(WorldSubsystem)));
		
		ASSERT_THAT(IsTrue(WorldSubsystem->bWasFlecsWorldInitialized));
		ASSERT_THAT(IsTrue(IsValid(WorldSubsystem->GetFlecsWorld())));

		ASSERT_THAT(IsTrue(WorldSubsystem->TimesChecked == 0));
		++WorldSubsystem->TimesChecked;
	}

	TEST_METHOD(NetworkSubsystemSingleton_IsAvailableWhenAbstractSubsystemsAreInitialized)
	{
		const UTestFlecsWorldSubsystem_Initialization* WorldSubsystem
			= World()->GetWorld()->GetSubsystem<UTestFlecsWorldSubsystem_Initialization>();
		ASSERT_THAT(IsTrue(IsValid(WorldSubsystem)));

		ASSERT_THAT(IsTrue(WorldSubsystem->bWasFlecsWorldInitialized));
		ASSERT_THAT(IsFalse(WorldSubsystem->bWasNetworkSubsystemSingletonAvailable));
	}
	
}; // FlecsWorldSubsystems

#endif // WITH_AUTOMATION_TESTS
