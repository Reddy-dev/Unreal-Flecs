// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Components/FlecsSubEntityRecordNameComponent.h"
#include "EntityRecords/FlecsEntityRecord.h"
#include "EntityRecords/FlecsNamedEntityRecordFragment.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsEntityRecordApplicationTests, "UnrealFlecs.EntityRecords.Application",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
			| EAutomationTestFlags::CriticalPriority, "[Flecs]")
{
protected:
	virtual void OnRegisteredWorldSetUp() override
	{
		World()->RegisterComponentType<EFlecsTestEnum_UENUM>();
		World()->RegisterComponentType<FFlecsTestStruct_Tag_Inherited>();
		World()->RegisterComponentType<FFlecsTestStruct_WithPropertyTraits>();
	}

public:
	TEST_METHOD(ApplyRecord_AddsScriptStructComponent)
	{
		FFlecsEntityRecord Record;
		Record.AddComponent<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ .Value = 123 });

		const FFlecsEntityHandle Entity = World()->CreateEntity();
		Record.ApplyRecordToEntity(World(), Entity);

		ASSERT_THAT(IsTrue(Entity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(Entity.Has(FFlecsTestStruct_Value::StaticStruct())));

		const auto& [Value] = Entity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Value == 123));

		const FFlecsTestStruct_Value* ValueByUStruct
			= static_cast<const FFlecsTestStruct_Value*>(Entity.TryGet(FFlecsTestStruct_Value::StaticStruct()));
		ASSERT_THAT(IsTrue(ValueByUStruct != nullptr));
		ASSERT_THAT(IsTrue(ValueByUStruct->Value == 123));
	}

	TEST_METHOD(ApplyRecord_AddsTagComponent_And_GameplayTag)
	{
		FFlecsEntityRecord Record;
		Record.AddComponent<FFlecsTestStruct_Tag>();
		Record.AddComponent(FFlecsTestNativeGameplayTags::Get().TestTag2);

		const FFlecsEntityHandle Entity = World()->CreateEntity();
		Record.ApplyRecordToEntity(World(), Entity);

		ASSERT_THAT(IsTrue(Entity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(Entity.Has(FFlecsTestStruct_Tag::StaticStruct())));
		ASSERT_THAT(IsTrue(Entity.Has(FFlecsTestNativeGameplayTags::Get().TestTag2)));
	}

	TEST_METHOD(ApplyRecord_AddsPairComponents_Tags)
	{
		FFlecsEntityRecord Record;
		
		FFlecsRecordPair Pair;
		Pair.First = FFlecsRecordPairSlot::Make<FUSTRUCTPairTestComponent>();
		Pair.Second = FFlecsRecordPairSlot::Make<FUSTRUCTPairTestComponent_Second>();
		Pair.PairValueType = EFlecsValuePairType::None;
		Record.AddComponent(MoveTemp(Pair));

		const FFlecsEntityHandle Entity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(Entity.IsValid()));
		
		Record.ApplyRecordToEntity(World(), Entity);
		ASSERT_THAT(IsTrue(Entity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
		ASSERT_THAT(IsTrue(Entity.HasPair(FUSTRUCTPairTestComponent::StaticStruct(), FUSTRUCTPairTestComponent_Second::StaticStruct())));
		
		ASSERT_THAT(IsFalse(Entity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Data>()));
	}

	TEST_METHOD(ApplyRecord_AddsPairComponents_Data_First)
	{
		FFlecsEntityRecord Record;
		
		FFlecsRecordPair Pair;
		Pair.First = FFlecsRecordPairSlot::Make<FUSTRUCTPairTestComponent_Data>(FUSTRUCTPairTestComponent_Data{ .Value = 123 });
		Pair.Second = FFlecsRecordPairSlot::Make<FUSTRUCTPairTestComponent>();
		Pair.PairValueType = EFlecsValuePairType::First;
		Record.AddComponent(MoveTemp(Pair));

		const FFlecsEntityHandle Entity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(Entity.IsValid()));

		Record.ApplyRecordToEntity(World(), Entity);
		ASSERT_THAT(IsTrue(Entity.HasPair<FUSTRUCTPairTestComponent_Data, FUSTRUCTPairTestComponent>()));
		ASSERT_THAT(IsTrue(Entity.HasPair(FUSTRUCTPairTestComponent_Data::StaticStruct(), FUSTRUCTPairTestComponent::StaticStruct())));
		
		ASSERT_THAT(IsFalse(Entity.HasPair<FUSTRUCTPairTestComponent_Second, FUSTRUCTPairTestComponent>()));
		ASSERT_THAT(IsFalse(Entity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Data>()));

		const FUSTRUCTPairTestComponent_Data& Data
			= Entity.GetPairFirst<FUSTRUCTPairTestComponent_Data, FUSTRUCTPairTestComponent>();

		ASSERT_THAT(IsTrue(Data.Value == 123));
	}

	TEST_METHOD(ApplyRecord_AddsPairComponents_Data_Second)
	{
		FFlecsEntityRecord Record;
		
		FFlecsRecordPair Pair;
		Pair.First = FFlecsRecordPairSlot::Make<FUSTRUCTPairTestComponent>();
		Pair.Second = FFlecsRecordPairSlot::Make<FUSTRUCTPairTestComponent_Data>(FUSTRUCTPairTestComponent_Data{ .Value = 456 });
		Pair.PairValueType = EFlecsValuePairType::Second;
		Record.AddComponent(MoveTemp(Pair));

		const FFlecsEntityHandle Entity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(Entity.IsValid()));
		
		Record.ApplyRecordToEntity(World(), Entity);
		ASSERT_THAT(IsTrue(Entity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Data>()));
		ASSERT_THAT(IsTrue(Entity.HasPair(FUSTRUCTPairTestComponent::StaticStruct(), FUSTRUCTPairTestComponent_Data::StaticStruct())));
		
		ASSERT_THAT(IsFalse(Entity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));

		const FUSTRUCTPairTestComponent_Data& Data
			= Entity.GetPairSecond<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Data>();
		
		ASSERT_THAT(IsTrue(Data.Value == 456));
	}

	TEST_METHOD(ApplyRecord_NoComponents_DoesNothing)
	{
		FFlecsEntityRecord Record;

		const FFlecsEntityHandle Entity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(Entity.IsValid()));
		
		Record.ApplyRecordToEntity(World(), Entity);

		ASSERT_THAT(IsFalse(Entity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsFalse(Entity.Has(FFlecsTestStruct_Tag::StaticStruct())));
		ASSERT_THAT(IsFalse(Entity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsFalse(Entity.Has(FFlecsTestStruct_Value::StaticStruct())));
		
		ASSERT_THAT(IsFalse(Entity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
	}

	TEST_METHOD(ApplyRecord_AddsMultipleComponents)
	{
		FFlecsEntityRecord Record;
		Record.AddComponent<FFlecsTestStruct_Tag>();
		Record.AddComponent<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ .Value = 789 });
		
		FFlecsRecordPair Pair;
		Pair.First = FFlecsRecordPairSlot::Make<FUSTRUCTPairTestComponent>();
		Pair.Second = FFlecsRecordPairSlot::Make<FUSTRUCTPairTestComponent_Second>();
		Pair.PairValueType = EFlecsValuePairType::None;
		Record.AddComponent(MoveTemp(Pair));

		const FFlecsEntityHandle Entity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(Entity.IsValid()));
		
		Record.ApplyRecordToEntity(World(), Entity);

		ASSERT_THAT(IsTrue(Entity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(Entity.Has(FFlecsTestStruct_Tag::StaticStruct())));
		ASSERT_THAT(IsTrue(Entity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(Entity.Has(FFlecsTestStruct_Value::StaticStruct())));
		
		const auto& [Value] = Entity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Value == 789));
		ASSERT_THAT(IsTrue(Entity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
	}

	TEST_METHOD(ApplyRecord_AddScriptEnum_ScriptAPI)
	{
		FFlecsEntityRecord Record;
		const FSolidEnumSelector EnumValue = FSolidEnumSelector::Make<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One);
		Record.AddComponent(EnumValue);

		const FFlecsEntityHandle Entity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(Entity.IsValid()));
		
		Record.ApplyRecordToEntity(World(), Entity);

		ASSERT_THAT(IsTrue(Entity.HasPair<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		ASSERT_THAT(IsTrue(Entity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::One)));
		ASSERT_THAT(IsTrue(Entity.Has(StaticEnum<EFlecsTestEnum_UENUM>(),
			static_cast<int64>(EFlecsTestEnum_UENUM::One))));
	}
	
	TEST_METHOD(ApplyRecord_AddScriptEnum_CPPAPI)
	{
		FFlecsEntityRecord Record;;
		Record.AddComponent<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Two);

		const FFlecsEntityHandle Entity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(Entity.IsValid()));
		
		Record.ApplyRecordToEntity(World(), Entity);

		ASSERT_THAT(IsTrue(Entity.HasPair<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		ASSERT_THAT(IsTrue(Entity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Two)));
		ASSERT_THAT(IsTrue(Entity.Has(StaticEnum<EFlecsTestEnum_UENUM>(),
			static_cast<int64>(EFlecsTestEnum_UENUM::Two))));
	}

	TEST_METHOD(ApplyRecord_WithSubEntities_AddsComponents)
	{
		FFlecsEntityRecord SubRecord;
		SubRecord.AddComponent<FFlecsTestStruct_Tag>();

		FFlecsEntityRecord Record;
		Record.AddSubEntity(SubRecord);

		const FFlecsEntityHandle Entity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(Entity.IsValid()));

		Record.ApplyRecordToEntity(World(), Entity);
		// Should be in the sub-entity
		ASSERT_THAT(IsFalse(Entity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsFalse(Entity.Has(FFlecsTestStruct_Tag::StaticStruct())));

		bool bFoundChild = false;
		Entity.IterateChildren([&](const FFlecsEntityHandle& ChildEntity)
		{
			if (ChildEntity.Has<FFlecsTestStruct_Tag>())
			{
				bFoundChild = true;
			}
		});

		ASSERT_THAT(IsTrue(bFoundChild));
	}
	
	TEST_METHOD(ApplyRecord_WithScriptStructTagsAndComponents_AddsAll)
	{
		FFlecsEntityRecord Record;
		Record.AddComponent(FFlecsTestStruct_Tag::StaticStruct());
		Record.AddComponent(FInstancedStruct::Make<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ .Value = 987 }));

		const FFlecsEntityHandle Entity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(Entity.IsValid()));

		Record.ApplyRecordToEntity(World(), Entity);

		ASSERT_THAT(IsTrue(Entity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(Entity.Has(FFlecsTestStruct_Tag::StaticStruct())));
		
		ASSERT_THAT(IsTrue(Entity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(Entity.Has(FFlecsTestStruct_Value::StaticStruct())));

		const auto& [Value] = Entity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Value == 987));
	}

}; // FlecsEntityRecordApplicationTests

#endif // WITH_AUTOMATION_TESTS
