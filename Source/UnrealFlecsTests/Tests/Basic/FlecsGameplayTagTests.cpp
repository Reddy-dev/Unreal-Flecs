// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsTestCase.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Worlds/FlecsWorld.h"

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(UnrealFlecsGameplayTagTests,
								   "UnrealFlecs.Entities.GameplayTags",
							   EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
							   | EAutomationTestFlags::CriticalPriority,
							   "[Flecs][Entity][Tag][GameplayTag]")
{
	TEST_METHOD(GameplayTagEntity_GetTagEntityAndTagFromEntity)
	{
		const FGameplayTag TestTag = FFlecsTestNativeGameplayTags::Get().TestTag1;
		ASSERT_THAT(IsTrue(TestTag.IsValid()));

		const FFlecsEntityHandle TagEntity = World()->GetTagEntity(TestTag);
		ASSERT_THAT(IsTrue(TagEntity.IsValid()));
		ASSERT_THAT(IsTrue(TagEntity.Has<FGameplayTag>()));

		const FGameplayTag RetrievedTag = TagEntity.Get<FGameplayTag>();
		ASSERT_THAT(IsTrue(RetrievedTag == TestTag));
	}

	TEST_METHOD(GameplayTagAddRemove_TagDepthMatrix)
	{
		struct FTagDepthCase
		{
			FGameplayTag Tag;
			FGameplayTag OtherTag;
		};

		const FFlecsTestNativeGameplayTags& Tags = FFlecsTestNativeGameplayTags::Get();
		const TArray<FFlecsNamedTestCase<FTagDepthCase>> Cases =
		{
			{ TEXT("OneLevelTag"), { Tags.TestTag1, Tags.TestTag2 } },
			{ TEXT("TwoLevelTag"), { Tags.TestTag2, Tags.TestTag1 } },
		};

		ForEachFlecsTestCase(*this, MakeArrayView(Cases),
			[this](const FFlecsNamedTestCase<FTagDepthCase>& TestCase)
			{
				ASSERT_THAT(IsTrue(TestCase.Value.Tag.IsValid()));
				const FFlecsEntityHandle Entity = World()->CreateEntity();
				ASSERT_THAT(IsTrue(Entity.IsValid()));

				ASSERT_THAT(IsFalse(Entity.Has(TestCase.Value.Tag)));
				Entity.Add(TestCase.Value.Tag);
				ASSERT_THAT(IsTrue(Entity.Has(TestCase.Value.Tag)));
				ASSERT_THAT(IsFalse(Entity.Has(TestCase.Value.OtherTag)));

				Entity.Remove(TestCase.Value.Tag);
				ASSERT_THAT(IsFalse(Entity.Has(TestCase.Value.Tag)));
				ASSERT_THAT(IsFalse(Entity.Has(TestCase.Value.OtherTag)));
				Entity.Destroy();
			});
	}

	TEST_METHOD(GameplayTagAddRemove_MultipleTags)
	{
		const FGameplayTag TestTag1 = FFlecsTestNativeGameplayTags::Get().TestTag1;
		const FGameplayTag TestTag2 = FFlecsTestNativeGameplayTags::Get().TestTag2;
		ASSERT_THAT(IsTrue(TestTag1.IsValid()));
		ASSERT_THAT(IsTrue(TestTag2.IsValid()));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.Add(TestTag1);
		TestEntity.Add(TestTag2);
		ASSERT_THAT(IsTrue(TestEntity.Has(TestTag1)));
		ASSERT_THAT(IsTrue(TestEntity.Has(TestTag2)));

		TestEntity.Remove(TestTag1);
		ASSERT_THAT(IsFalse(TestEntity.Has(TestTag1)));
		ASSERT_THAT(IsTrue(TestEntity.Has(TestTag2)));

		TestEntity.Remove(TestTag2);
		ASSERT_THAT(IsFalse(TestEntity.Has(TestTag1)));
		ASSERT_THAT(IsFalse(TestEntity.Has(TestTag2)));
	}

	TEST_METHOD(GameplayTagAddRemove_Pair_TagRel_ValueTarget)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Value>();
		
		const FGameplayTag TestTag1 = FFlecsTestNativeGameplayTags::Get().TestTag1;
		const FGameplayTag TestTag2 = FFlecsTestNativeGameplayTags::Get().TestTag2;
		ASSERT_THAT(IsTrue(TestTag1.IsValid()));
		ASSERT_THAT(IsTrue(TestTag2.IsValid()));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.SetPairSecond<FFlecsTestStruct_Value>(TestTag1, FFlecsTestStruct_Value{ 55 });
		ASSERT_THAT(IsTrue(TestEntity.HasPairSecond<FFlecsTestStruct_Value>(TestTag1)));
		
		const FFlecsTestStruct_Value& RetrievedValue = TestEntity.GetPairSecond<FFlecsTestStruct_Value>(TestTag1);
		ASSERT_THAT(IsTrue(RetrievedValue.Value == 55));

		TestEntity.RemovePairSecond<FFlecsTestStruct_Value>(TestTag1);
		ASSERT_THAT(IsFalse(TestEntity.HasPairSecond<FFlecsTestStruct_Value>(TestTag1)));
	}

	TEST_METHOD(GameplayTagAddRemove_Pair_TagRel_TagTarget)
	{
		const FGameplayTag TestTag1 = FFlecsTestNativeGameplayTags::Get().TestTag1;
		const FGameplayTag TestTag2 = FFlecsTestNativeGameplayTags::Get().TestTag2;
		ASSERT_THAT(IsTrue(TestTag1.IsValid()));
		ASSERT_THAT(IsTrue(TestTag2.IsValid()));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddPair(TestTag1, TestTag2);
		ASSERT_THAT(IsTrue(TestEntity.HasPair(TestTag1, TestTag2)));

		TestEntity.RemovePair(TestTag1, TestTag2);
		ASSERT_THAT(IsFalse(TestEntity.HasPair(TestTag1, TestTag2)));
	}

	TEST_METHOD(GameplayTagAddRemove_Pair_TagRel_EntityTarget)
	{
		const FGameplayTag TestTag1 = FFlecsTestNativeGameplayTags::Get().TestTag1;
		ASSERT_THAT(IsTrue(TestTag1.IsValid()));

		const FFlecsEntityHandle TargetEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TargetEntity.IsValid()));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddPair(TestTag1, TargetEntity);
		ASSERT_THAT(IsTrue(TestEntity.HasPair(TestTag1, TargetEntity)));

		TestEntity.RemovePair(TestTag1, TargetEntity);
		ASSERT_THAT(IsFalse(TestEntity.HasPair(TestTag1, TargetEntity)));
	}

	TEST_METHOD(GameplayTagAddRemove_Pair_ValueRel_TagTarget)
	{
		World()->RegisterComponentType<FFlecsTestStruct_Value>();
		
		const FGameplayTag TestTag2 = FFlecsTestNativeGameplayTags::Get().TestTag2;
		ASSERT_THAT(IsTrue(TestTag2.IsValid()));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.SetPair<FFlecsTestStruct_Value>(TestTag2, FFlecsTestStruct_Value{ 42 });
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FFlecsTestStruct_Value>(TestTag2)));
		
		const FFlecsTestStruct_Value& RetrievedValue = TestEntity.GetPairFirst<FFlecsTestStruct_Value>(TestTag2);
		ASSERT_THAT(IsTrue(RetrievedValue.Value == 42));

		TestEntity.RemovePair<FFlecsTestStruct_Value>(TestTag2);
		ASSERT_THAT(IsFalse(TestEntity.HasPair<FFlecsTestStruct_Value>(TestTag2)));
	}
	
}; // UnrealFlecsGameplayTagTests

#endif // #if WITH_AUTOMATION_TESTS
