// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Components/FlecsSubEntityRecordNameComponent.h"
#include "EntityRecords/FlecsEntityRecord.h"
#include "EntityRecords/FlecsNamedEntityRecordFragment.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsEntityRecordPrefabTests, "UnrealFlecs.EntityRecords.Prefabs",
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
	TEST_METHOD(CreatePrefabWithRecord_ApplyPrefabToEntity_AddsTagComponent)
	{
		FFlecsEntityRecord Record;
		Record.AddComponent<FFlecsTestStruct_Tag>();

		const FFlecsEntityHandle PrefabEntity = World()->CreatePrefabWithRecord(Record);
		ASSERT_THAT(IsTrue(PrefabEntity.IsValid()));
		ASSERT_THAT(IsTrue(PrefabEntity.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddPrefab(PrefabEntity);
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Tag::StaticStruct())));
		
		ASSERT_THAT(IsTrue(TestEntity.IsA(PrefabEntity)));
	}

	TEST_METHOD(CreatePrefabWithRecord_ApplyPrefabToEntity_AddsScriptStructComponent)
	{
		FFlecsEntityRecord Record;
		Record.AddComponent<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ .Value = 321 });

		const FFlecsEntityHandle PrefabEntity = World()->CreatePrefabWithRecord(Record);
		ASSERT_THAT(IsTrue(PrefabEntity.IsValid()));
		ASSERT_THAT(IsTrue(PrefabEntity.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddPrefab(PrefabEntity);
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));

		const auto& [Value] = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Value == 321));
	}

	TEST_METHOD(CreatePrefabWithRecord_ApplyPrefabToEntity_AddsPairComponents)
	{
		FFlecsEntityRecord Record;
		
		FFlecsRecordPair Pair;
		Pair.First = FFlecsRecordPairSlot::Make<FUSTRUCTPairTestComponent>();
		Pair.Second = FFlecsRecordPairSlot::Make<FUSTRUCTPairTestComponent_Second>();
		Pair.PairValueType = EFlecsValuePairType::None;
		Record.AddComponent(MoveTemp(Pair));

		const FFlecsEntityHandle PrefabEntity = World()->CreatePrefabWithRecord(Record);
		ASSERT_THAT(IsTrue(PrefabEntity.IsValid()));
		ASSERT_THAT(IsTrue(PrefabEntity.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddPrefab(PrefabEntity);
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
	}

	TEST_METHOD(CreatePrefabWithRecord_ApplyPrefabToEntity_MultipleComponents)
	{
		FFlecsEntityRecord Record;
		Record.AddComponent<FFlecsTestStruct_Tag>();
		Record.AddComponent<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ .Value = 654 });
		
		FFlecsRecordPair Pair;
		Pair.First = FFlecsRecordPairSlot::Make<FUSTRUCTPairTestComponent>();
		Pair.Second = FFlecsRecordPairSlot::Make<FUSTRUCTPairTestComponent_Second>();
		Pair.PairValueType = EFlecsValuePairType::None;
		Record.AddComponent(MoveTemp(Pair));

		const FFlecsEntityHandle PrefabEntity = World()->CreatePrefabWithRecord(Record);
		ASSERT_THAT(IsTrue(PrefabEntity.IsValid()));
		ASSERT_THAT(IsTrue(PrefabEntity.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddPrefab(PrefabEntity);

		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Tag::StaticStruct())));
		ASSERT_THAT(IsTrue(TestEntity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(TestEntity.Has(FFlecsTestStruct_Value::StaticStruct())));
		
		const auto& [Value] = TestEntity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Value == 654));
		ASSERT_THAT(IsTrue(TestEntity.HasPair<FUSTRUCTPairTestComponent, FUSTRUCTPairTestComponent_Second>()));
	}

	TEST_METHOD(CreatePrefabWithRecord_ApplyPrefabToEntity_AddsScriptEnum)
	{
		FFlecsEntityRecord Record;
		const FSolidEnumSelector EnumValue = FSolidEnumSelector::Make<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Two);
		Record.AddComponent(EnumValue);

		const FFlecsEntityHandle PrefabEntity = World()->CreatePrefabWithRecord(Record);
		ASSERT_THAT(IsTrue(PrefabEntity.IsValid()));
		ASSERT_THAT(IsTrue(PrefabEntity.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddPrefab(PrefabEntity);

		ASSERT_THAT(IsTrue(TestEntity.HasPair<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		ASSERT_THAT(IsTrue(TestEntity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Two)));
		ASSERT_THAT(IsTrue(TestEntity.Has(StaticEnum<EFlecsTestEnum_UENUM>(),
			static_cast<int64>(EFlecsTestEnum_UENUM::Two))));
	}

	TEST_METHOD(CreatePrefabWithRecord_WithSubEntities_ApplyPrefabToEntity_AddsComponentsToSubEntities)
	{
		FFlecsEntityRecord SubRecord;
		SubRecord.AddComponent<FFlecsTestStruct_Tag>();

		FFlecsEntityRecord Record;
		Record.AddSubEntity(SubRecord);

		const FFlecsEntityHandle PrefabEntity = World()->CreatePrefabWithRecord(Record);
		ASSERT_THAT(IsTrue(PrefabEntity.IsValid()));
		ASSERT_THAT(IsTrue(PrefabEntity.Has(flecs::Prefab)));

		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));

		TestEntity.AddPrefab(PrefabEntity);

		// Should be in the sub-entity
		ASSERT_THAT(IsFalse(TestEntity.Has<FFlecsTestStruct_Tag>()));

		bool bFoundChild = false;
		TestEntity.IterateChildren([&](const FFlecsEntityHandle& ChildEntity)
		{
			if (ChildEntity.Has<FFlecsTestStruct_Tag>())
			{
				bFoundChild = true;
			}
		});

		ASSERT_THAT(IsTrue(bFoundChild));
	}

}; // FlecsEntityRecordPrefabTests

#endif // WITH_AUTOMATION_TESTS
