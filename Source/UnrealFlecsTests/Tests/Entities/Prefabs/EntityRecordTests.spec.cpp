﻿// Elie Wiese-Namir © 2025. All Rights Reserved.


#if WITH_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Fixtures/FlecsWorldFixture.h"
#include "EntityRecordTestStructs.h"

BEGIN_DEFINE_SPEC(FEntityRecordTestsSpec,
                  "Flecs.Entity.Record",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

FFlecsTestFixture Fixture;

END_DEFINE_SPEC(FEntityRecordTestsSpec);

void FEntityRecordTestsSpec::Define()
{
	BeforeEach([this]()
	{
		Fixture.SetUp();

		Fixture.FlecsWorld->RegisterComponentType<FTestStruct_EntityRecord>();
	});
	
	AfterEach([this]()
	{
		Fixture.TearDown();
	});

	Describe("Test Creating and Applying Entity Records", [this]()
	{
		It("Should create a new entity record", [this]()
		{
			FFlecsEntityRecord Record;
			
			Record.AddComponent<FTestStruct_EntityRecord>(FTestStruct_EntityRecord{ 1 });

			TestTrue("Record should have the component", !Record.Components.IsEmpty());
		});

		It("Should create an entity with a record", [this]()
		{
			FFlecsEntityRecord Record;
			
			Record.AddComponent<FTestStruct_EntityRecord>(FTestStruct_EntityRecord{ 1 });

			FFlecsEntityHandle TestEntity = Fixture.FlecsWorld->CreateEntityWithRecord(Record);
			TestTrue("Entity should be valid", TestEntity.IsValid());
			TestTrue("Entity should have the record", TestEntity.Has<FTestStruct_EntityRecord>());
			TestEqual("Entity should have the correct record",
				TestEntity.Get<FTestStruct_EntityRecord>().IntegerValue, 1);
		});

		It("Should apply a record to an entity", [this]()
		{
			FFlecsEntityRecord Record;
			
			Record.AddComponent<FTestStruct_EntityRecord>(FTestStruct_EntityRecord{ 1 });

			FFlecsEntityHandle TestEntity = Fixture.FlecsWorld->CreateEntity();
			TestTrue("Entity should be valid", TestEntity.IsValid());

			Record.ApplyRecordToEntity(TestEntity);
			TestTrue("Entity should have the record",
				TestEntity.Has<FTestStruct_EntityRecord>());
			TestEqual("Entity should have the correct record",
				TestEntity.Get<FTestStruct_EntityRecord>().IntegerValue, 1);
		});
	});
}

#endif // WITH_AUTOMATION_TESTS
