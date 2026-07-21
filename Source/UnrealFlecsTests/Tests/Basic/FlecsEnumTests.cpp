// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Worlds/FlecsWorld.h"

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(UnrealFlecsEnumRegistrationTests,
								 "UnrealFlecs.Components.Enums.Registration",
                               EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
                               | EAutomationTestFlags::CriticalPriority,
                               "[Flecs][Component][Pair][Enum][CPP-API][StaticEnum-API]")
{
	TEST_METHOD_WITH_TAGS(RegisterStaticEnumAPI_AddRemoveReplaceStaticEnumAPI,
								   "[Flecs][Component][Enum][StaticEnum-API]")
	{
		const FFlecsComponentHandle TestEnumEntity = World()->RegisterComponentType(StaticEnum<EFlecsTestEnum_UENUM>());
		ASSERT_THAT(IsTrue(TestEnumEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEnumEntity.IsComponent()));
		ASSERT_THAT(IsTrue(TestEnumEntity.IsEnum()));
		
		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		
		const FFlecsEntityHandle SymbolLookupEntity = World()->LookupEntityBySymbol_Internal("EFlecsTestEnum_UENUM");
		ASSERT_THAT(IsTrue(SymbolLookupEntity.IsValid()));
		ASSERT_THAT(AreEqual(SymbolLookupEntity, TestEnumEntity));
		
		const FFlecsEntityHandle AliasLookupEntity = World()->LookupEntity("UEnum_EFlecsTestEnum_UENUM");
		ASSERT_THAT(IsTrue(AliasLookupEntity.IsValid()));
		ASSERT_THAT(AreEqual(AliasLookupEntity, TestEnumEntity));

		// Add the enum value using StaticEnum API
		TestEntity.Add(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<uint64>(EFlecsTestEnum_UENUM::One));
		ASSERT_THAT(IsTrue(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>())));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		ASSERT_THAT(IsTrue(TestEntity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One)));
		ASSERT_THAT(IsTrue(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::One))));

		const EFlecsTestEnum_UENUM CPPEnumValue = TestEntity.GetEnumValue<EFlecsTestEnum_UENUM>();
		ASSERT_THAT(AreEqual(CPPEnumValue, EFlecsTestEnum_UENUM::One));

		const EFlecsTestEnum_UENUM StaticEnumValue = static_cast<EFlecsTestEnum_UENUM>(
			TestEntity.GetEnumValue(StaticEnum<EFlecsTestEnum_UENUM>()));
		ASSERT_THAT(AreEqual(StaticEnumValue, EFlecsTestEnum_UENUM::One));

		// Replace the enum value with a different one
		TestEntity.Add(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<uint64>(EFlecsTestEnum_UENUM::Three));
		ASSERT_THAT(IsTrue(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>())));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		ASSERT_THAT(IsTrue(TestEntity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Three)));
		ASSERT_THAT(IsTrue(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::Three))));
		
		ASSERT_THAT(IsFalse(TestEntity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One)));
		ASSERT_THAT(IsFalse(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::One))));
		
		// Remove the enum value using StaticEnum API
		TestEntity.Remove(StaticEnum<EFlecsTestEnum_UENUM>());
		ASSERT_THAT(IsFalse(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>())));
		ASSERT_THAT(IsFalse(TestEntity.HasPair<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		ASSERT_THAT(IsFalse(TestEntity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One)));
		ASSERT_THAT(IsFalse(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::One))));
	}

	TEST_METHOD_WITH_TAGS(RegisterCPPAPI_AddRemoveReplaceCPPAPI,
								   "[Flecs][Component][Enum][CPP-API]")
	{
		const FFlecsComponentHandle TestEnumEntity = World()->RegisterComponentType<EFlecsTestEnum_UENUM>();
		ASSERT_THAT(IsTrue(TestEnumEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEnumEntity.IsComponent()));
		ASSERT_THAT(IsTrue(TestEnumEntity.IsEnum()));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		
		const FFlecsEntityHandle SymbolLookupEntity = World()->LookupEntityBySymbol_Internal("EFlecsTestEnum_UENUM");
		ASSERT_THAT(IsTrue(SymbolLookupEntity.IsValid()));
		ASSERT_THAT(AreEqual(SymbolLookupEntity, TestEnumEntity));
		
		const FFlecsEntityHandle AliasLookupEntity = World()->LookupEntity("UEnum_EFlecsTestEnum_UENUM");
		ASSERT_THAT(IsTrue(AliasLookupEntity.IsValid()));
		ASSERT_THAT(AreEqual(AliasLookupEntity, TestEnumEntity));

		TestEntity.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One);
		ASSERT_THAT(IsTrue(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>())));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		ASSERT_THAT(IsTrue(TestEntity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One)));
		ASSERT_THAT(IsTrue(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::One))));

		const EFlecsTestEnum_UENUM CPPEnumValue = TestEntity.GetEnumValue<EFlecsTestEnum_UENUM>();
		ASSERT_THAT(AreEqual(CPPEnumValue, EFlecsTestEnum_UENUM::One));
		const EFlecsTestEnum_UENUM StaticEnumValue = static_cast<EFlecsTestEnum_UENUM>(
			TestEntity.GetEnumValue(StaticEnum<EFlecsTestEnum_UENUM>()));
		ASSERT_THAT(AreEqual(StaticEnumValue, EFlecsTestEnum_UENUM::One));

		TestEntity.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Three);
		ASSERT_THAT(IsTrue(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>())));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		ASSERT_THAT(IsTrue(TestEntity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Three)));
		ASSERT_THAT(IsTrue(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::Three))));
		
		ASSERT_THAT(IsFalse(TestEntity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One)));
		ASSERT_THAT(IsFalse(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::One))));
		
		TestEntity.Remove<EFlecsTestEnum_UENUM>();
		ASSERT_THAT(IsFalse(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>())));
		ASSERT_THAT(IsFalse(TestEntity.HasPair<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		ASSERT_THAT(IsFalse(TestEntity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One)));
		ASSERT_THAT(IsFalse(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::One))));
	}

	TEST_METHOD_WITH_TAGS(RegisterStaticEnumAPI_Add_CPPAPI_Remove_StaticEnumAPI_Replace_StaticEnumAPI,
			"[Flecs][Component][Enum][StaticEnum-API]")
	{ 
		const FFlecsComponentHandle TestEnumEntity = World()->RegisterComponentType(StaticEnum<EFlecsTestEnum_UENUM>());
		ASSERT_THAT(IsTrue(TestEnumEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEnumEntity.IsComponent()));
		ASSERT_THAT(IsTrue(TestEnumEntity.IsEnum()));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		
		const FFlecsEntityHandle SymbolLookupEntity = World()->LookupEntityBySymbol_Internal("EFlecsTestEnum_UENUM");
		ASSERT_THAT(IsTrue(SymbolLookupEntity.IsValid()));
		ASSERT_THAT(AreEqual(SymbolLookupEntity, TestEnumEntity));

		TestEntity.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One);
		ASSERT_THAT(IsTrue(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>())));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		ASSERT_THAT(IsTrue(TestEntity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One)));
		ASSERT_THAT(IsTrue(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::One))));

		const EFlecsTestEnum_UENUM CPPEnumValue = TestEntity.GetEnumValue<EFlecsTestEnum_UENUM>();
		ASSERT_THAT(AreEqual(CPPEnumValue, EFlecsTestEnum_UENUM::One));
		const EFlecsTestEnum_UENUM StaticEnumValue = static_cast<EFlecsTestEnum_UENUM>(
			TestEntity.GetEnumValue(StaticEnum<EFlecsTestEnum_UENUM>()));
		ASSERT_THAT(AreEqual(StaticEnumValue, EFlecsTestEnum_UENUM::One));

		TestEntity.Add(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<uint64>(EFlecsTestEnum_UENUM::Three));
		ASSERT_THAT(IsTrue(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>())));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		ASSERT_THAT(IsTrue(TestEntity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Three)));
		ASSERT_THAT(IsTrue(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::Three))));
		
		ASSERT_THAT(IsFalse(TestEntity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One)));
		ASSERT_THAT(IsFalse(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::One))));
		
		TestEntity.Remove(StaticEnum<EFlecsTestEnum_UENUM>());
		ASSERT_THAT(IsFalse(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>())));
		ASSERT_THAT(IsFalse(TestEntity.HasPair<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		ASSERT_THAT(IsFalse(TestEntity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One)));
		ASSERT_THAT(IsFalse(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::One))));
	}

	TEST_METHOD_WITH_TAGS(RegisterCPPAPI_AddRemoveStaticEnumAPI_Replace_StaticCPPAPI,
			"[Flecs][Component][Enum][CPP-API]")
	{
		const FFlecsComponentHandle TestEnumEntity = World()->RegisterComponentType<EFlecsTestEnum_UENUM>();
		ASSERT_THAT(IsTrue(TestEnumEntity.IsValid()));
		ASSERT_THAT(IsTrue(TestEnumEntity.IsComponent()));
		ASSERT_THAT(IsTrue(TestEnumEntity.IsEnum()));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		
		const FFlecsEntityHandle SymbolLookupEntity = World()->LookupEntityBySymbol_Internal("EFlecsTestEnum_UENUM");
		ASSERT_THAT(IsTrue(SymbolLookupEntity.IsValid()));
		ASSERT_THAT(AreEqual(SymbolLookupEntity, TestEnumEntity));

		TestEntity.Add(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<uint64>(EFlecsTestEnum_UENUM::One));
		ASSERT_THAT(IsTrue(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>())));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		ASSERT_THAT(IsTrue(TestEntity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One)));
		ASSERT_THAT(IsTrue(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::One))));

		const EFlecsTestEnum_UENUM CPPEnumValue = TestEntity.GetEnumValue<EFlecsTestEnum_UENUM>();
		ASSERT_THAT(AreEqual(CPPEnumValue, EFlecsTestEnum_UENUM::One));
		const EFlecsTestEnum_UENUM StaticEnumValue = static_cast<EFlecsTestEnum_UENUM>(
			TestEntity.GetEnumValue(StaticEnum<EFlecsTestEnum_UENUM>()));
		ASSERT_THAT(AreEqual(StaticEnumValue, EFlecsTestEnum_UENUM::One));

		TestEntity.Add<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Three);
		ASSERT_THAT(IsTrue(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>())));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		ASSERT_THAT(IsTrue(TestEntity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Three)));
		ASSERT_THAT(IsTrue(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::Three))));
		
		ASSERT_THAT(IsFalse(TestEntity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One)));
		ASSERT_THAT(IsFalse(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::One))));

		TestEntity.Remove<EFlecsTestEnum_UENUM>();
		ASSERT_THAT(IsFalse(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>())));
		ASSERT_THAT(IsFalse(TestEntity.HasPair<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		ASSERT_THAT(IsFalse(TestEntity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One)));
		ASSERT_THAT(IsFalse(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::One))));
	}

}; // UnrealFlecsEnumRegistrationTests

#endif // #if WITH_AUTOMATION_TESTS
