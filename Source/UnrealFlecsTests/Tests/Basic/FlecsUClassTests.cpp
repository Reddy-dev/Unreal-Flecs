// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "CQTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsWorldFixture.h"
#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Tests/Types/FlecsClassTestTypes.h"
#include "Worlds/FlecsWorld.h"

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsUClassTests, "UnrealFlecs.Types.UClass",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
	| EAutomationTestFlags::CriticalPriority, "[Flecs]")
{
	TEST_METHOD(ShouldRegisterUClassAsType_CPPAPI)
	{
		const FFlecsEntityHandle CPPTypedEntity = World()->ObtainTypedEntity<UFlecsUObjectComponentTestObject>();
		ASSERT_THAT(IsTrue(CPPTypedEntity.IsValid()));

		const FFlecsEntityHandle ScriptClassTypedEntity = World()->ObtainTypedEntity(UFlecsUObjectComponentTestObject::StaticClass());
		ASSERT_THAT(IsTrue(ScriptClassTypedEntity.IsValid()));

		ASSERT_THAT(IsTrue(CPPTypedEntity == ScriptClassTypedEntity,
			TEXT("TypedEntity should be equal to ScriptClassTypedEntity after creation")));
	}

	TEST_METHOD(ShouldRegisterUClassAsType_ScriptClassAPI)
	{
		const FFlecsEntityHandle ScriptClassTypedEntity = World()->ObtainTypedEntity(UFlecsUObjectComponentTestObject::StaticClass());
		ASSERT_THAT(IsTrue(ScriptClassTypedEntity.IsValid()));

		const FFlecsEntityHandle CPPTypedEntity = World()->ObtainTypedEntity<UFlecsUObjectComponentTestObject>();
		ASSERT_THAT(IsTrue(CPPTypedEntity.IsValid()));

		ASSERT_THAT(IsTrue(ScriptClassTypedEntity == CPPTypedEntity,
			TEXT("ScriptClassTypedEntity should be equal to CPPTypedEntity after creation")));
	}
	
}; // FlecsUClassTests

#endif // WITH_AUTOMATION_TESTS
