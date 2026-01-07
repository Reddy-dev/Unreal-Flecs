// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#include "Queries/FlecsQueryDefinition.h"

/**
 * Layout of the tests:
 * A. Construction Tests
 **/
TEST_CLASS_WITH_FLAGS(B2_UnrealFlecsQueryDefinitionTests,
							   "UnrealFlecs.B2_Query_Definition",
							   EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
								| EAutomationTestFlags::CriticalPriority)
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
	}

	AFTER_ALL()
	{
		Fixture.Reset();
	}
	
	TEST_METHOD(A1_DefaultConstruction)
	{
		FFlecsQueryDefinition QueryDefinition;

		ASSERT_THAT(AreEqual(QueryDefinition.CacheType, EFlecsQueryCacheType::Default));
		ASSERT_THAT(IsFalse(QueryDefinition.bDetectChanges));
		ASSERT_THAT(AreEqual(QueryDefinition.Flags, static_cast<uint8>(EFlecsQueryFlags::None)));
		ASSERT_THAT(AreEqual(QueryDefinition.Terms.Num(), 0));
		ASSERT_THAT(AreEqual(QueryDefinition.OtherExpressions.Num(), 0));
		
		flecs::query_builder<> QueryBuilder(FlecsWorld->World);
		QueryDefinition.Apply(FlecsWorld, QueryBuilder);
		flecs::query<> Query = QueryBuilder.build();
		
		ASSERT_THAT(IsNotNull(Query.c_ptr()));
	}

}; // End of B2_UnrealFlecsQueryDefinitionTests



#endif // WITH_AUTOMATION_TESTS
