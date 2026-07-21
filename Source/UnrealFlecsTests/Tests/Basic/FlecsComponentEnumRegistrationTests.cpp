// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Worlds/FlecsWorld.h"

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsComponentEnumRegistrationTests,
								   "UnrealFlecs.Components.EnumRegistration",
							   EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
								| EAutomationTestFlags::CriticalPriority,
							   "[Flecs][Entity][Tag][Component][Registration]")
{
	TEST_METHOD(EnumComponentRegistration_CPPAPI)
	{
		const FFlecsEntityHandle EnumEntity = World()->RegisterComponentType<EFlecsTestEnum_UENUM>();
		ASSERT_THAT(IsTrue(EnumEntity.IsValid()));

		ASSERT_THAT(IsTrue(EnumEntity.IsComponent()));
		ASSERT_THAT(IsTrue(EnumEntity.IsEnum()));

		const FFlecsEntityHandle StaticEnumEntity = World()->RegisterComponentType(StaticEnum<EFlecsTestEnum_UENUM>());
		ASSERT_THAT(IsTrue(StaticEnumEntity.IsValid()));
		ASSERT_THAT(IsTrue(StaticEnumEntity.IsComponent()));
		ASSERT_THAT(IsTrue(StaticEnumEntity.IsEnum()));

		ASSERT_THAT(IsTrue(EnumEntity == StaticEnumEntity));
	}

	TEST_METHOD(EnumComponentRegistration_StaticStructAPI)
	{
		const FFlecsEntityHandle StaticEnumEntity = World()->RegisterComponentType(StaticEnum<EFlecsTestEnum_UENUM>());
		ASSERT_THAT(IsTrue(StaticEnumEntity.IsValid()));
		
		ASSERT_THAT(IsTrue(StaticEnumEntity.IsComponent()));
		ASSERT_THAT(IsTrue(StaticEnumEntity.IsEnum()));

		const FFlecsEntityHandle EnumEntity = World()->RegisterComponentType<EFlecsTestEnum_UENUM>();
		ASSERT_THAT(IsTrue(EnumEntity.IsValid()));

		ASSERT_THAT(IsTrue(EnumEntity.IsComponent()));
		ASSERT_THAT(IsTrue(EnumEntity.IsEnum()));

		ASSERT_THAT(IsTrue(StaticEnumEntity == EnumEntity));
	}

	TEST_METHOD(SparseEnumComponentRegistration_CPPAPI)
	{
		const FFlecsEntityHandle SparseEnumEntity = World()->RegisterComponentType<EFlecsTestEnum_UENUM>();
		ASSERT_THAT(IsTrue(SparseEnumEntity.IsValid()));

		ASSERT_THAT(IsTrue(SparseEnumEntity.IsComponent()));
		ASSERT_THAT(IsTrue(SparseEnumEntity.IsEnum()));

		const FFlecsEntityHandle StaticSparseEnumEntity = World()->RegisterComponentType(StaticEnum<EFlecsTestEnum_UENUM>());
		ASSERT_THAT(IsTrue(StaticSparseEnumEntity.IsValid()));
		
		ASSERT_THAT(IsTrue(StaticSparseEnumEntity.IsComponent()));
		ASSERT_THAT(IsTrue(StaticSparseEnumEntity.IsEnum()));

		ASSERT_THAT(IsTrue(SparseEnumEntity == StaticSparseEnumEntity));
	}

	TEST_METHOD(SparseEnumComponentRegistration_StaticStructAPI)
	{
		const FFlecsEntityHandle StaticSparseEnumEntity = World()->RegisterComponentType(StaticEnum<EFlecsTestEnum_UENUM>());
		ASSERT_THAT(IsTrue(StaticSparseEnumEntity.IsValid()));

		ASSERT_THAT(IsTrue(StaticSparseEnumEntity.IsComponent()));
		ASSERT_THAT(IsTrue(StaticSparseEnumEntity.IsEnum()));

		const FFlecsEntityHandle SparseEnumEntity = World()->RegisterComponentType<EFlecsTestEnum_UENUM>();
		ASSERT_THAT(IsTrue(SparseEnumEntity.IsValid()));

		ASSERT_THAT(IsTrue(SparseEnumEntity.IsComponent()));
		ASSERT_THAT(IsTrue(SparseEnumEntity.IsEnum()));

		ASSERT_THAT(IsTrue(StaticSparseEnumEntity == SparseEnumEntity));
	}

}; // FlecsComponentEnumRegistrationTests


#endif // #if WITH_AUTOMATION_TESTS
