// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "CQTest.h"

#include "UnrealFlecsConfigMacros.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Entities/FlecsId.h"
#include "UObject/UnrealType.h"

#if WITH_EDITOR
#include "EdGraphSchema_K2.h"
#endif

TEST_CLASS_WITH_FLAGS_AND_TAGS(
	FlecsIdSerializationTests,
	"UnrealFlecs.Types.FlecsIdSerialization",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter,
	"[Flecs][Types][Serialization]")
{
	TEST_METHOD(ExportText_AppendsCanonicalValue)
	{
		const FFlecsId Id(123456789ULL);
		FString ExportedValue = TEXT("Prefix:");

		ASSERT_THAT(IsTrue(Id.ExportTextItem(
			ExportedValue,
			FFlecsId::Null(),
			nullptr,
			0,
			nullptr)));
		ASSERT_THAT(IsTrue(ExportedValue == TEXT("Prefix:FlecsId=123456789")));
	}

	TEST_METHOD(ImportText_AcceptsBlueprintFormatsAndAdvancesBuffer)
	{
		const TCHAR* Formats[] = {
			TEXT("FlecsId=123456789,Next"),
			TEXT("(Id=123456789),Next"),
			TEXT("(FlecsId=123456789),Next"),
			TEXT("123456789,Next"),
		};

		for (const TCHAR* Format : Formats)
		{
			const TCHAR* Buffer = Format;
			FFlecsId Id;

			ASSERT_THAT(IsTrue(Id.ImportTextItem(Buffer, 0, nullptr, nullptr)));
			ASSERT_THAT(IsTrue(Id.GetId() == 123456789ULL));
			ASSERT_THAT(IsTrue(FCString::Strcmp(Buffer, TEXT(",Next")) == 0));
		}
	}

#if WITH_EDITOR
	TEST_METHOD(ReflectedStorage_CanBeEmittedByBlueprintBytecode)
	{
		const FInt64Property* IdProperty = FindFProperty<FInt64Property>(
			FFlecsId::StaticStruct(),
			GET_MEMBER_NAME_CHECKED(FFlecsId, Id));

		ASSERT_THAT(IsTrue(IdProperty != nullptr));

		FEdGraphPinType PinType;
		ASSERT_THAT(IsTrue(GetDefault<UEdGraphSchema_K2>()->ConvertPropertyToPinType(
			IdProperty,
			PinType)));
		ASSERT_THAT(IsTrue(PinType.PinCategory == UEdGraphSchema_K2::PC_Int64));
	}
#endif

	TEST_METHOD(ScriptStructImport_PreservesPairBitPattern)
	{
		constexpr uint64 PairId = 0x8000000100000002ULL;
		FFlecsId Id;

		const TCHAR* Result = FFlecsId::StaticStruct()->ImportText(
			TEXT("FlecsId=9223372041149743106"),
			&Id,
			nullptr,
			PPF_None,
			nullptr,
			TEXT("FlecsId"));

		ASSERT_THAT(IsTrue(Result != nullptr));
		ASSERT_THAT(IsTrue(Id.GetId() == PairId));

		const FInt64Property* IdProperty = FindFProperty<FInt64Property>(
			FFlecsId::StaticStruct(),
			GET_MEMBER_NAME_CHECKED(FFlecsId, Id));
		ASSERT_THAT(IsTrue(IdProperty != nullptr));

		FString MemberLiteral;
		IdProperty->ExportText_InContainer(
			0,
			MemberLiteral,
			&Id,
			&Id,
			nullptr,
			PPF_None);

		int64 EmittedValue = 0;
		LexFromString(EmittedValue, *MemberLiteral);
		ASSERT_THAT(IsTrue(std::bit_cast<uint64>(EmittedValue) == PairId));
	}
};

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS
