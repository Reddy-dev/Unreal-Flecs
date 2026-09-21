// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsWorldFixture.h"
#include "UnrealFlecsTests/Tests/Types/FlecsRegisteredObjectDependencyTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Worlds/FlecsWorld.h"

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsRegisteredObjectDependencyTests,
	"UnrealFlecs.Objects.RegistrationDependencies",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
		| EAutomationTestFlags::CriticalPriority,
	"[Flecs][Registration][Objects][Dependencies]")
{
	TEST_METHOD(MissingDependency_DefersRegistration)
	{
		UFlecsRegisteredObjectDependentTestObject* DependentObject
			= World()->RegisterFlecsObject<UFlecsRegisteredObjectDependentTestObject>();

		ASSERT_THAT(IsNull(DependentObject));
		ASSERT_THAT(IsFalse(World()->IsFlecsObjectRegistered<UFlecsRegisteredObjectDependentTestObject>()));
		ASSERT_THAT(IsFalse(World()->IsFlecsObjectRegistered<UFlecsRegisteredObjectDependencyTestObject>()));
	}

	TEST_METHOD(RegisteringDependency_RegistersDeferredDependent)
	{
		World()->RegisterFlecsObject<UFlecsRegisteredObjectDependentTestObject>();

		UFlecsRegisteredObjectDependencyTestObject* DependencyObject
			= World()->RegisterFlecsObject<UFlecsRegisteredObjectDependencyTestObject>();
		UFlecsRegisteredObjectDependentTestObject* DependentObject
			= World()->GetRegisteredFlecsObject<UFlecsRegisteredObjectDependentTestObject>();

		ASSERT_THAT(IsNotNull(DependencyObject));
		ASSERT_THAT(IsNotNull(DependentObject));
		ASSERT_THAT(AreEqual(1, DependencyObject->GetRegisterObjectCallCount()));
		ASSERT_THAT(AreEqual(1, DependentObject->GetRegisterObjectCallCount()));
		ASSERT_THAT(IsTrue(DependentObject->WasDependencyRegistered()));
	}

	TEST_METHOD(RegisteringDependency_ResumesTransitiveDependents)
	{
		World()->RegisterFlecsObject<UFlecsRegisteredObjectTransitiveDependentTestObject>();
		World()->RegisterFlecsObject<UFlecsRegisteredObjectDependentTestObject>();

		UFlecsRegisteredObjectDependencyTestObject* DependencyObject
			= World()->RegisterFlecsObject<UFlecsRegisteredObjectDependencyTestObject>();
		UFlecsRegisteredObjectDependentTestObject* DependentObject
			= World()->GetRegisteredFlecsObject<UFlecsRegisteredObjectDependentTestObject>();
		UFlecsRegisteredObjectTransitiveDependentTestObject* TransitiveDependentObject
			= World()->GetRegisteredFlecsObject<UFlecsRegisteredObjectTransitiveDependentTestObject>();

		ASSERT_THAT(IsNotNull(DependencyObject));
		ASSERT_THAT(IsNotNull(DependentObject));
		ASSERT_THAT(IsNotNull(TransitiveDependentObject));
		ASSERT_THAT(AreEqual(1, DependencyObject->GetRegisterObjectCallCount()));
		ASSERT_THAT(AreEqual(1, DependentObject->GetRegisterObjectCallCount()));
		ASSERT_THAT(AreEqual(1, TransitiveDependentObject->GetRegisterObjectCallCount()));
		ASSERT_THAT(IsTrue(DependentObject->WasDependencyRegistered()));
		ASSERT_THAT(IsTrue(TransitiveDependentObject->WasDependencyRegistered()));
	}
}; // FlecsRegisteredObjectDependencyTests

#endif // #if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS
