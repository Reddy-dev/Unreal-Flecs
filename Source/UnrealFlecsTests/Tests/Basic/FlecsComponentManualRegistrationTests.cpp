// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Worlds/FlecsWorld.h"

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsComponentManualRegistrationTests,
								   "UnrealFlecs.Components.ManualRegistration",
							   EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
								| EAutomationTestFlags::CriticalPriority,
							   "[Flecs][Entity][Tag][Component][Registration]")
{
	TEST_METHOD(RegisterScriptStructOneByteOneUProperty_CPPAPI)
	{
		const FFlecsEntityHandle OneByteOneUPropertyEntity = World()->RegisterComponentType<FUStructTestComponent_NonTagUSTRUCT>();
		ASSERT_THAT(IsTrue(OneByteOneUPropertyEntity.IsValid()));

		ASSERT_THAT(IsTrue(OneByteOneUPropertyEntity.IsComponent()));
		ASSERT_THAT(IsFalse(OneByteOneUPropertyEntity.IsTag()));
	}

	TEST_METHOD(RegisterScriptStructOneByteOneUProperty_StaticStructAPI)
	{
		const FFlecsEntityHandle OneByteOneUPropertyEntity = World()->RegisterComponentType(FUStructTestComponent_NonTagUSTRUCT::StaticStruct());
		ASSERT_THAT(IsTrue(OneByteOneUPropertyEntity.IsValid()));

		ASSERT_THAT(IsTrue(OneByteOneUPropertyEntity.IsComponent()));
		ASSERT_THAT(IsFalse(OneByteOneUPropertyEntity.IsTag()));
	}

	TEST_METHOD(RegisterScriptStructOneByteWithoutUProperty_CPPAPI)
	{
		const FFlecsEntityHandle OneByteWithoutUPropertyEntity = World()->RegisterComponentType<FUSTructTestComponent_AccidentalTag>();
		ASSERT_THAT(IsTrue(OneByteWithoutUPropertyEntity.IsValid()));

		ASSERT_THAT(IsTrue(OneByteWithoutUPropertyEntity.IsComponent()));
		ASSERT_THAT(IsFalse(OneByteWithoutUPropertyEntity.IsTag()));
	}

	TEST_METHOD(RegisterScriptStructOneByteWithoutUProperty_StaticStructAPI)
	{
		const FFlecsEntityHandle OneByteWithoutUPropertyEntity = World()->RegisterComponentType(FUSTructTestComponent_AccidentalTag::StaticStruct());
		ASSERT_THAT(IsTrue(OneByteWithoutUPropertyEntity.IsValid()));

		ASSERT_THAT(IsTrue(OneByteWithoutUPropertyEntity.IsComponent()));
		// @TODO: Can we unify this?
		ASSERT_THAT(IsTrue(OneByteWithoutUPropertyEntity.IsTag()));
	}
	
	TEST_METHOD(UnrealVariantTypeBidrectional_RegisterCPPAPI_CPPAPI)
	{
		const UScriptStruct* ScriptStruct = TBaseStructure<FMatrix>::Get();
		
		const FFlecsEntityHandle StructEntity = World()->RegisterComponentType<FMatrix>();
		ASSERT_THAT(IsTrue(StructEntity.IsValid()));
		
		const FFlecsEntityHandle StaticStructEntity = World()->RegisterComponentType(ScriptStruct);
		ASSERT_THAT(IsTrue(StaticStructEntity.IsValid()));
		ASSERT_THAT(IsTrue(StaticStructEntity == StructEntity));
		
		const FFlecsEntityHandle SymbolLookupResult = World()->LookupEntityBySymbol_Internal(TEXT("FMatrix"));
		ASSERT_THAT(IsTrue(SymbolLookupResult.IsValid()));
		ASSERT_THAT(IsTrue(SymbolLookupResult == StructEntity));
		
		const FFlecsEntityHandle ScriptStructLookupResult = World()->GetScriptStructEntity(ScriptStruct);
		ASSERT_THAT(IsTrue(ScriptStructLookupResult.IsValid()));
		ASSERT_THAT(IsTrue(ScriptStructLookupResult == StructEntity));
	}
	
	TEST_METHOD(UnrealVariantTypeBidrectional_RegisterStaticStructAPI_StaticStructAPI)
	{
		const UScriptStruct* ScriptStruct = TBaseStructure<FMatrix>::Get();
		
		const FFlecsEntityHandle StructEntity = World()->RegisterComponentType(ScriptStruct);
		ASSERT_THAT(IsTrue(StructEntity.IsValid()));
		
		const FFlecsEntityHandle CPPRegister = World()->RegisterComponentType<FMatrix>();
		ASSERT_THAT(IsTrue(CPPRegister.IsValid()));
		ASSERT_THAT(IsTrue(CPPRegister == StructEntity));
		
		const FFlecsEntityHandle SymbolLookupResult = World()->LookupEntityBySymbol_Internal(TEXT("FMatrix"));
		ASSERT_THAT(IsTrue(SymbolLookupResult.IsValid()));
		ASSERT_THAT(IsTrue(SymbolLookupResult == StructEntity));
		
		const FFlecsEntityHandle ScriptStructLookupResult = World()->GetScriptStructEntity(ScriptStruct);
		ASSERT_THAT(IsTrue(ScriptStructLookupResult.IsValid()));
		ASSERT_THAT(IsTrue(ScriptStructLookupResult == StructEntity));
	}
	
	
}; // FlecsComponentManualRegistrationTests


#endif // #if WITH_AUTOMATION_TESTS
