// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "CQTest.h"
#include "Misc/AutomationTest.h"

#include "UnrealFlecsConfigMacros.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Networking/DefaultFlecsNetworkIdGenerator.h"

TEST_CLASS_WITH_FLAGS_AND_TAGS(DefaultFlecsNetworkIdGeneratorTests,
	"UnrealFlecs.Networking.NetworkIdGenerator",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter,
	"[Flecs][Networking]")
{
	TEST_METHOD(GenerateNetworkId_MultipleIds_UsesSequentialSlotsAndInitialGeneration)
	{
		UDefaultFlecsNetworkIdGenerator* Generator = NewObject<UDefaultFlecsNetworkIdGenerator>();

		const FFlecsNetworkId FirstId = Generator->GenerateNetworkId();
		const FFlecsNetworkId SecondId = Generator->GenerateNetworkId();
		const FFlecsNetworkId ThirdId = Generator->GenerateNetworkId();

		ASSERT_THAT(IsTrue(FirstId.IsValid()));
		ASSERT_THAT(IsTrue(SecondId.IsValid()));
		ASSERT_THAT(IsTrue(ThirdId.IsValid()));
		
		ASSERT_THAT(AreEqual(static_cast<uint32>(0), FirstId.GetSlot()));
		ASSERT_THAT(AreEqual(static_cast<uint32>(1), SecondId.GetSlot()));
		ASSERT_THAT(AreEqual(static_cast<uint32>(2), ThirdId.GetSlot()));
		ASSERT_THAT(AreEqual(static_cast<uint32>(1), FirstId.GetGeneration()));
		ASSERT_THAT(AreEqual(static_cast<uint32>(1), SecondId.GetGeneration()));
		ASSERT_THAT(AreEqual(static_cast<uint32>(1), ThirdId.GetGeneration()));
	}

	TEST_METHOD(ReleaseNetworkId_AllocatedId_ReusesSlotWithIncrementedGeneration)
	{
		UDefaultFlecsNetworkIdGenerator* Generator = NewObject<UDefaultFlecsNetworkIdGenerator>();
		const FFlecsNetworkId ReleasedId = Generator->GenerateNetworkId();

		ASSERT_THAT(IsTrue(Generator->ReleaseNetworkId(ReleasedId)));

		const FFlecsNetworkId ReusedId = Generator->GenerateNetworkId();

		ASSERT_THAT(AreEqual(ReleasedId.GetSlot(), ReusedId.GetSlot()));
		ASSERT_THAT(AreEqual(ReleasedId.GetGeneration() + 1, ReusedId.GetGeneration()));
		
		ASSERT_THAT(IsFalse(ReleasedId == ReusedId));
	}

	TEST_METHOD(ReleaseNetworkId_ReusedId_IncrementsGenerationAgain)
	{
		UDefaultFlecsNetworkIdGenerator* Generator = NewObject<UDefaultFlecsNetworkIdGenerator>();
		const FFlecsNetworkId FirstId = Generator->GenerateNetworkId();
		Generator->ReleaseNetworkId(FirstId);

		const FFlecsNetworkId SecondId = Generator->GenerateNetworkId();
		Generator->ReleaseNetworkId(SecondId);

		const FFlecsNetworkId ThirdId = Generator->GenerateNetworkId();

		ASSERT_THAT(AreEqual(FirstId.GetSlot(), ThirdId.GetSlot()));
		ASSERT_THAT(AreEqual(FirstId.GetGeneration() + 2, ThirdId.GetGeneration()));
	}

	TEST_METHOD(ReleaseNetworkId_InvalidId_ReturnsFalseAndDoesNotQueueSlot)
	{
		UDefaultFlecsNetworkIdGenerator* Generator = NewObject<UDefaultFlecsNetworkIdGenerator>();

		ASSERT_THAT(IsFalse(Generator->ReleaseNetworkId(FFlecsNetworkId())));

		const FFlecsNetworkId FirstId = Generator->GenerateNetworkId();

		ASSERT_THAT(AreEqual(static_cast<uint32>(0), FirstId.GetSlot()));
		ASSERT_THAT(AreEqual(static_cast<uint32>(1), FirstId.GetGeneration()));
	}

	TEST_METHOD(ResetNetworkIdGenerator_PreviousStateIsCleared)
	{
		UDefaultFlecsNetworkIdGenerator* Generator = NewObject<UDefaultFlecsNetworkIdGenerator>();
		ASSERT_THAT(IsTrue(IsValid(Generator)));
		
		const FFlecsNetworkId FirstId = Generator->GenerateNetworkId();
		const FFlecsNetworkId ReleasedId = Generator->GenerateNetworkId();
		Generator->ReleaseNetworkId(ReleasedId);

		Generator->ResetNetworkIdGenerator();

		const FFlecsNetworkId IdAfterReset = Generator->GenerateNetworkId();

		ASSERT_THAT(AreEqual(FirstId.GetSlot(), IdAfterReset.GetSlot()));
		ASSERT_THAT(AreEqual(FirstId.GetGeneration(), IdAfterReset.GetGeneration()));
		ASSERT_THAT(AreEqual(FirstId.GetValue(), IdAfterReset.GetValue()));
	}
}; // TEST_CLASS DefaultFlecsNetworkIdGeneratorTests

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS
