// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Worlds/FlecsWorld.h"

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsComponentRegistrationTests,
								   "UnrealFlecs.Components.Registration",
							   EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
								| EAutomationTestFlags::CriticalPriority,
							   "[Flecs][Entity][Tag][Component][Registration]")
{
	TEST_METHOD(BasicUSTRUCTComponentRegistration_CPPAPI)
	{
		ASSERT_THAT(IsFalse(World()->HasScriptStruct(FFlecsTestStruct_Value::StaticStruct())));
		
		const FFlecsEntityHandle StructEntity = World()->RegisterComponentType<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(StructEntity.IsValid()));
		
		ASSERT_THAT(IsTrue(StructEntity.IsComponent()));
		ASSERT_THAT(IsFalse(StructEntity.IsTag()));
		
		const FFlecsEntityHandle StaticStructEntity = World()->RegisterComponentType(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsTrue(StaticStructEntity.IsValid()));

		ASSERT_THAT(IsTrue(StaticStructEntity.IsComponent()));
		ASSERT_THAT(IsTrue(StaticStructEntity.IsValueComponent()));
		ASSERT_THAT(IsFalse(StaticStructEntity.IsTag()));

		ASSERT_THAT(IsTrue(World()->IsIdType(StructEntity)));
		//ASSERT_THAT(IsTrue(World()->IsIdInUse(StructEntity)));
		ASSERT_THAT(IsFalse(World()->IsIdTag(StructEntity)));

		ASSERT_THAT(IsTrue(StructEntity == StaticStructEntity));
		
		const FString StructSymbolIdentifier = StructEntity.GetSymbol();
		ASSERT_THAT(IsTrue(StructSymbolIdentifier.Contains(TEXT("FFlecsTestStruct_Value"))));
		
		const FFlecsEntityHandle SymbolLookupResult = World()->LookupEntityBySymbol_Internal(TEXT("FFlecsTestStruct_Value"));
		ASSERT_THAT(IsTrue(SymbolLookupResult.IsValid()));
		ASSERT_THAT(IsTrue(SymbolLookupResult == StructEntity));
		
		const FFlecsEntityHandle AliasLookupResult = World()->LookupEntity(TEXT("UScriptStruct_FFlecsTestStruct_Value"));
		ASSERT_THAT(IsTrue(AliasLookupResult.IsValid()));
		ASSERT_THAT(IsTrue(AliasLookupResult == StructEntity));
	}

	TEST_METHOD(BasicUSTRUCTComponentRegistration_StaticStructAPI)
	{
		const FFlecsEntityHandle StaticStructEntity = World()->RegisterComponentType(FFlecsTestStruct_Value::StaticStruct());
		ASSERT_THAT(IsTrue(StaticStructEntity.IsValid()));

		ASSERT_THAT(IsTrue(StaticStructEntity.IsComponent()));
		ASSERT_THAT(IsFalse(StaticStructEntity.IsTag()));
		
		const FFlecsEntityHandle StructEntity = World()->RegisterComponentType<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(StructEntity.IsValid()));

		ASSERT_THAT(IsTrue(StructEntity.IsComponent()));
		ASSERT_THAT(IsTrue(StructEntity.IsValueComponent()));
		ASSERT_THAT(IsFalse(StructEntity.IsTag()));

		ASSERT_THAT(IsTrue(World()->IsIdType(StructEntity)));
		//ASSERT_THAT(IsTrue(World()->IsIdInUse(StructEntity)));
		ASSERT_THAT(IsFalse(World()->IsIdTag(StructEntity)));

		ASSERT_THAT(IsTrue(StaticStructEntity == StructEntity));
	}

	TEST_METHOD(BasicComponentRegistration_CPPAPI)
	{
		const FFlecsEntityHandle StructEntity = World()->RegisterComponentType<FFlecsTest_CPPStructValue>();
		ASSERT_THAT(IsTrue(StructEntity.IsValid()));

		ASSERT_THAT(IsTrue(StructEntity.IsComponent()));
		ASSERT_THAT(IsFalse(StructEntity.IsTag()));
		
		ASSERT_THAT(IsTrue(World()->IsIdType(StructEntity)));
		//ASSERT_THAT(IsTrue(World()->IsIdInUse(StructEntity)));
		ASSERT_THAT(IsFalse(World()->IsIdTag(StructEntity)));
		ASSERT_THAT(IsTrue(StructEntity.IsValueComponent()));
		ASSERT_THAT(IsFalse(StructEntity.IsTag()));
		
		const FString StructSymbolIdentifier = StructEntity.GetSymbol();
		ASSERT_THAT(IsTrue(StructSymbolIdentifier.Contains(TEXT("FFlecsTest_CPPStructValue"))));
		
		const FFlecsEntityHandle SymbolLookupResult = World()->LookupEntityBySymbol_Internal(TEXT("FFlecsTest_CPPStructValue"));
		ASSERT_THAT(IsTrue(SymbolLookupResult.IsValid()));
		ASSERT_THAT(IsTrue(SymbolLookupResult == StructEntity));
	}

	TEST_METHOD(BasicComponentRegistration_Tag_CPPAPI)
	{
		const FFlecsEntityHandle TagEntity = World()->RegisterComponentType<FFlecsTestStruct_Tag>();
		ASSERT_THAT(IsTrue(TagEntity.IsValid()));

		ASSERT_THAT(IsTrue(TagEntity.IsTag()));
		ASSERT_THAT(IsFalse(TagEntity.IsValueComponent()));
		
		ASSERT_THAT(IsFalse(World()->IsIdType(TagEntity)));
		//ASSERT_THAT(IsTrue(World()->IsIdInUse(TagEntity)));
		ASSERT_THAT(IsTrue(World()->IsIdTag(TagEntity)));
		
		const FFlecsEntityHandle SymbolLookupResult = World()->LookupEntityBySymbol_Internal(TEXT("FFlecsTestStruct_Tag"));
		ASSERT_THAT(IsTrue(SymbolLookupResult.IsValid()));
		ASSERT_THAT(IsTrue(SymbolLookupResult == TagEntity));
		
		const FFlecsEntityHandle AliasLookupResult = World()->LookupEntity(TEXT("UScriptStruct_FFlecsTestStruct_Tag"));
		ASSERT_THAT(IsTrue(AliasLookupResult.IsValid()));
		ASSERT_THAT(IsTrue(AliasLookupResult == TagEntity));
	}

	TEST_METHOD(BasicComponentRegistration_Tag_StaticStructAPI)
	{
		const FFlecsEntityHandle TagEntity = World()->RegisterComponentType(FFlecsTestStruct_Tag::StaticStruct());
		ASSERT_THAT(IsTrue(TagEntity.IsValid()));

		ASSERT_THAT(IsTrue(TagEntity.IsTag()));
		//ASSERT_THAT(IsFalse(TagEntity.IsComponent()));
		
		ASSERT_THAT(IsFalse(World()->IsIdType(TagEntity)));
		//ASSERT_THAT(IsTrue(World()->IsIdInUse(TagEntity)));
		ASSERT_THAT(IsTrue(World()->IsIdTag(TagEntity)));
		
		const FFlecsEntityHandle SymbolLookupResult = World()->LookupEntityBySymbol_Internal(TEXT("FFlecsTestStruct_Tag"));
		ASSERT_THAT(IsTrue(SymbolLookupResult.IsValid()));
		ASSERT_THAT(IsTrue(SymbolLookupResult == TagEntity));
		
		const FFlecsEntityHandle AliasLookupResult = World()->LookupEntity(TEXT("UScriptStruct_FFlecsTestStruct_Tag"));
		ASSERT_THAT(IsTrue(AliasLookupResult.IsValid()));
		ASSERT_THAT(IsTrue(AliasLookupResult == TagEntity));
	}

	TEST_METHOD(BasicComponentRegistration_Tag_CPPOnlyType)
	{
		const FFlecsEntityHandle TagEntity = World()->RegisterComponentType<FFlecsTest_CPPStruct>();
		ASSERT_THAT(IsTrue(TagEntity.IsValid()));

		ASSERT_THAT(IsTrue(TagEntity.IsTag()));
		//ASSERT_THAT(IsFalse(TagEntity.IsComponent()));
		
		ASSERT_THAT(IsFalse(World()->IsIdType(TagEntity)));
		//ASSERT_THAT(IsTrue(World()->IsIdInUse(TagEntity)));
		ASSERT_THAT(IsTrue(World()->IsIdTag(TagEntity)));
		ASSERT_THAT(IsTrue(TagEntity.IsTag()));
		ASSERT_THAT(IsFalse(TagEntity.IsValueComponent()));
		
		const FString TagSymbolIdentifier = TagEntity.GetSymbol();
		ASSERT_THAT(IsTrue(TagSymbolIdentifier.Contains(TEXT("FFlecsTest_CPPStruct"))));
		
		const FFlecsEntityHandle SymbolLookupResult = World()->LookupEntityBySymbol_Internal(TEXT("FFlecsTest_CPPStruct"));
		ASSERT_THAT(IsTrue(SymbolLookupResult.IsValid()));
		ASSERT_THAT(IsTrue(SymbolLookupResult == TagEntity));
	}

}; // FlecsComponentRegistrationTests


#endif // #if WITH_AUTOMATION_TESTS
