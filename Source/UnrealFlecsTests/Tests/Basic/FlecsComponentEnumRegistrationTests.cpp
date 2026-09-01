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

	TEST_METHOD(EnumComponentRegistration_ScriptUENUM_MapsUnderlyingType)
	{
		const UEnum* ScriptEnum = StaticEnum<EFlecsTestEnum_UENUM_Int32>();
		ASSERT_THAT(IsTrue(IsValid(ScriptEnum)));
		ASSERT_THAT(IsTrue(ScriptEnum->GetUnderlyingType() == UEnum::EUnderlyingType::int32));

		const FFlecsEntityHandle EnumEntity = World()->RegisterComponentType(ScriptEnum);
		ASSERT_THAT(IsTrue(EnumEntity.IsValid()));
		ASSERT_THAT(IsTrue(EnumEntity.IsComponent()));
		ASSERT_THAT(IsTrue(EnumEntity.IsEnum()));
		ASSERT_THAT(AreEqual(EnumEntity.Get<flecs::Enum>().underlying_type, flecs::I32));
	}
	
	TEST_METHOD(EnumComponentRegistration_CPPUENUM_MapsUnderlyingType)
	{
		const FFlecsEntityHandle EnumEntity = World()->RegisterComponentType<EFlecsTestEnum_UENUM_Int32>();
		ASSERT_THAT(IsTrue(EnumEntity.IsValid()));
		ASSERT_THAT(IsTrue(EnumEntity.IsComponent()));
		ASSERT_THAT(IsTrue(EnumEntity.IsEnum()));
		ASSERT_THAT(AreEqual(EnumEntity.Get<flecs::Enum>().underlying_type, flecs::I32));
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

	TEST_METHOD(EnumComponentRegistration_MapsUnderlyingType)
	{
		struct FUnderlyingTypeTestCase
		{
			UEnum::EUnderlyingType UnrealType;
			flecs::entity_t FlecsType;
		}; // struct FUnderlyingTypeTestCase

		const TArray<FUnderlyingTypeTestCase> TestCases =
		{
			{ UEnum::EUnderlyingType::int8, flecs::I8 },
			{ UEnum::EUnderlyingType::int16, flecs::I16 },
			{ UEnum::EUnderlyingType::int32, flecs::I32 },
			{ UEnum::EUnderlyingType::int64, flecs::I64 },
			{ UEnum::EUnderlyingType::uint8, flecs::U8 },
			{ UEnum::EUnderlyingType::uint16, flecs::U16 },
			{ UEnum::EUnderlyingType::uint32, flecs::U32 },
			{ UEnum::EUnderlyingType::uint64, flecs::U64 },
		};

		for (int32 TestCaseIndex = 0; TestCaseIndex < TestCases.Num(); ++TestCaseIndex)
		{
			const FUnderlyingTypeTestCase& TestCase = TestCases[TestCaseIndex];
			const FName EnumName(*FString::Printf(TEXT("FlecsTestUnderlyingEnum_%d"), TestCaseIndex));
			UEnum* TestEnum = NewObject<UEnum>(GetTransientPackage(), EnumName, RF_Transient);
			ASSERT_THAT(IsTrue(IsValid(TestEnum)));

			TArray<TPair<FName, int64>> EnumValues =
			{
				{ FName(TEXT("None")), 0 },
				{ FName(TEXT("Value")), 1 },
			};
			ASSERT_THAT(IsTrue(TestEnum->SetEnums(EnumValues,
				UEnum::ECppForm::EnumClass,
				TestCase.UnrealType,
				EEnumFlags::None,
				UEnum::EAddMaxKeyIfMissing::No)));

			const FFlecsEntityHandle EnumEntity = World()->RegisterComponentType(TestEnum);
			ASSERT_THAT(IsTrue(EnumEntity.IsValid()));
			ASSERT_THAT(IsTrue(EnumEntity.IsEnum()));
			ASSERT_THAT(AreEqual(EnumEntity.Get<flecs::Enum>().underlying_type, TestCase.FlecsType));
		}
	}

}; // FlecsComponentEnumRegistrationTests


#endif // #if WITH_AUTOMATION_TESTS
