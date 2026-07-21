// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Components/FlecsSubEntityRecordNameComponent.h"
#include "EntityRecords/FlecsEntityRecord.h"
#include "EntityRecords/FlecsNamedEntityRecordFragment.h"

FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsEntityRecordBuilderTests, "UnrealFlecs.EntityRecords.Builder",
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
	TEST_METHOD(BuilderAPI_CreateEntityRecord_AddsComponents_CPPAPI)
	{
		FFlecsEntityRecord Record = FFlecsEntityRecord().Builder()
			.Fragment<FFlecsNamedEntityRecordFragment>("BuilderAPITestEntity")
			.Component<FFlecsTestStruct_Tag>()
			.Component<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ .Value = 987 })
			.Component(FFlecsTestStruct_WithPropertyTraits::StaticStruct())
			.GameplayTag(FFlecsTestNativeGameplayTags::Get().TestTag1)
			.Enum(EFlecsTestEnum_UENUM::Two)
			.Build();

		const FFlecsEntityHandle Entity = World()->CreateEntityWithRecord(Record);
		ASSERT_THAT(IsTrue(Entity.IsValid()));
		
		ASSERT_THAT(IsTrue(Entity.HasName()));
		ASSERT_THAT(AreEqual(TEXT("BuilderAPITestEntity"), Entity.GetName()));
		
		ASSERT_THAT(IsTrue(Entity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(Entity.Has(FFlecsTestStruct_Tag::StaticStruct())));
		
		ASSERT_THAT(IsTrue(Entity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(Entity.Has(FFlecsTestStruct_Value::StaticStruct())));
		
		ASSERT_THAT(IsTrue(Entity.Has(FFlecsTestStruct_WithPropertyTraits::StaticStruct())));
		
		ASSERT_THAT(IsTrue(Entity.Has(FFlecsTestNativeGameplayTags::Get().TestTag1)));
		
		ASSERT_THAT(IsTrue(Entity.HasPair<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		ASSERT_THAT(IsTrue(Entity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Two)));
		ASSERT_THAT(IsTrue(Entity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::Two))));
		
		const auto& [Value] = Entity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Value == 987));
	}
	
	TEST_METHOD(BuilderAPI_CreateEntityRecord_AddsComponents_ScriptStructAPI)
	{
		FFlecsEntityRecord Record = FFlecsEntityRecord().Builder()
			.Fragment<FFlecsNamedEntityRecordFragment>("BuilderAPITestEntity_ScriptStructAPI")
			.Component(FFlecsTestStruct_Tag::StaticStruct())
			.Component(FInstancedStruct::Make<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ .Value = 654 }))
			.GameplayTag(FFlecsTestNativeGameplayTags::Get().TestTag2)
			.Enum(FSolidEnumSelector::Make<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Three))
			.Build();

		const FFlecsEntityHandle Entity = World()->CreateEntityWithRecord(Record);
		ASSERT_THAT(IsTrue(Entity.IsValid()));
		
		ASSERT_THAT(IsTrue(Entity.HasName()));
		ASSERT_THAT(AreEqual(TEXT("BuilderAPITestEntity_ScriptStructAPI"), Entity.GetName()));
		
		ASSERT_THAT(IsTrue(Entity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(Entity.Has(FFlecsTestStruct_Tag::StaticStruct())));
		
		ASSERT_THAT(IsTrue(Entity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(Entity.Has(FFlecsTestStruct_Value::StaticStruct())));
		
		ASSERT_THAT(IsTrue(Entity.Has(FFlecsTestNativeGameplayTags::Get().TestTag2)));
		
		ASSERT_THAT(IsTrue(Entity.HasPair<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		ASSERT_THAT(IsTrue(Entity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Three)));
		ASSERT_THAT(IsTrue(Entity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::Three))));

		const auto& [Value] = Entity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Value == 654));
	}
	
	TEST_METHOD(BuilderAPI_CreateEntityRecord_UsingCustomFragmentBuilderAPI)
	{
		FFlecsEntityRecord Record = FFlecsEntityRecord().Builder()
			.FragmentScope<FFlecsNamedEntityRecordFragment>()
				.Named("BuilderAPITestEntity_CustomBuilder")
			.End()
			.Component(FFlecsTestStruct_Tag::StaticStruct())
			.Component(FInstancedStruct::Make<FFlecsTestStruct_Value>(FFlecsTestStruct_Value{ .Value = 654 }))
			.GameplayTag(FFlecsTestNativeGameplayTags::Get().TestTag2)
			.Enum(FSolidEnumSelector::Make<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Three))
			.Build();

		const FFlecsEntityHandle Entity = World()->CreateEntityWithRecord(Record);
		ASSERT_THAT(IsTrue(Entity.IsValid()));
		
		ASSERT_THAT(IsTrue(Entity.HasName()));
		ASSERT_THAT(AreEqual(TEXT("BuilderAPITestEntity_CustomBuilder"), Entity.GetName()));
		
		ASSERT_THAT(IsTrue(Entity.Has<FFlecsTestStruct_Tag>()));
		ASSERT_THAT(IsTrue(Entity.Has(FFlecsTestStruct_Tag::StaticStruct())));
		
		ASSERT_THAT(IsTrue(Entity.Has<FFlecsTestStruct_Value>()));
		ASSERT_THAT(IsTrue(Entity.Has(FFlecsTestStruct_Value::StaticStruct())));
		
		ASSERT_THAT(IsTrue(Entity.Has(FFlecsTestNativeGameplayTags::Get().TestTag2)));
		
		ASSERT_THAT(IsTrue(Entity.HasPair<EFlecsTestEnum_UENUM>(flecs::Wildcard)));
		ASSERT_THAT(IsTrue(Entity.Has<EFlecsTestEnum_UENUM>(EFlecsTestEnum_UENUM::Three)));
		ASSERT_THAT(IsTrue(Entity.Has(StaticEnum<EFlecsTestEnum_UENUM>(), static_cast<int64>(EFlecsTestEnum_UENUM::Three))));

		const auto& [Value] = Entity.Get<FFlecsTestStruct_Value>();
		ASSERT_THAT(IsTrue(Value == 654));
	}
	
}; // FlecsEntityRecordBuilderTests

#endif // WITH_AUTOMATION_TESTS
