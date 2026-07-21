// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Pipelines/FlecsTickTypeNativeTags.h"

#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldConverter.h"
#include "Worlds/UnrealFlecsWorldTag.h"

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsWorldTests, "UnrealFlecs.World.Lifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
	| EAutomationTestFlags::CriticalPriority,
	"[Flecs][OS-API][World][Entity]")
{
	TEST_METHOD(AllocateMemoryOSAPI)
	{
		static constexpr uint32 MemorySize = 16;
		
		void* Memory = ecs_os_malloc(MemorySize);
		ASSERT_THAT(IsNotNull(Memory));
		
		ecs_os_free(Memory);
		Memory = nullptr;
	}

	TEST_METHOD(ReAllocateMemoryOSAPI)
	{
		static constexpr uint32 MemorySize = 16;
		
		void* Memory = ecs_os_malloc(MemorySize);
		ASSERT_THAT(IsNotNull(Memory));
		
		void* ReallocatedMemory = ecs_os_realloc(Memory, MemorySize * 2);
		ASSERT_THAT(IsNotNull(ReallocatedMemory));
		
		ecs_os_free(ReallocatedMemory);
	}

	TEST_METHOD(CallocOSAPI)
	{
		static constexpr uint32 MemorySize = 16;
		
		void* Memory = ecs_os_calloc(MemorySize);
		ASSERT_THAT(IsNotNull(Memory));
		
		const TSolidNotNull<const uint8*> ByteMemory = static_cast<const uint8*>(Memory);
		
		for (uint32 Index = 0; Index < MemorySize; ++Index)
		{
			ASSERT_THAT(AreEqual(ByteMemory[Index], 0));
		}
		
		ecs_os_free(Memory);
	}

	TEST_METHOD(GetTimeNowOSAPI)
	{
		const uint32_t Time = ecs_os_now();
		ASSERT_THAT(IsTrue(Time > 0));
	}

	TEST_METHOD(SleepNanoSecondsOSAPI)
	{
		static FTimespan SleepTime = FTimespan::FromMilliseconds(10);
		
		const uint32 StartTime = ecs_os_now();
		ecs_os_sleep(SleepTime.GetSeconds(), 0);
		const uint32 EndTime = ecs_os_now();
		
		ASSERT_THAT(IsTrue(EndTime > StartTime));
	}
	
	TEST_METHOD(CanCreateWorld)
	{
		ASSERT_THAT(IsTrue(IsValid(World())));
	}

	TEST_METHOD(CanGetWorldEntity)
	{
		const FFlecsEntityHandle WorldEntity = World()->GetWorldEntity();
		ASSERT_THAT(IsTrue(WorldEntity.IsValid()));
		
		ASSERT_THAT(AreEqual(FString("World"), WorldEntity.GetName()));
	}

	TEST_METHOD(CanConvertFlecsWorldToUFlecsWorld)
	{
		const TSolidNotNull<UFlecsWorld*> ConvertedWorld = UE::Flecs::ToUnrealFlecsWorld(World()->GetNativeFlecsWorld());
		ASSERT_THAT(IsTrue(IsValid(ConvertedWorld)));
		
		ASSERT_THAT(IsTrue(World() == ConvertedWorld));
	}

	TEST_METHOD(IsUnrealFlecsWorld)
	{
		ASSERT_THAT(IsTrue(World()->Has<FUnrealFlecsWorldTag>()));

		flecs::world non_unreal_world;
		non_unreal_world.component<FUnrealFlecsWorldTag>();
		ASSERT_THAT(IsFalse(non_unreal_world.has<FUnrealFlecsWorldTag>()));
	}

	TEST_METHOD(CanCreateEntity)
	{
		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
	}

	TEST_METHOD(CanCreateEntityWithID)
	{
		static constexpr FFlecsId TestId = FLECS_HI_COMPONENT_ID + 10012;
		
		const FFlecsEntityHandle EntityWithId = World()->CreateEntityWithId(TestId);
		ASSERT_THAT(IsTrue(EntityWithId.IsValid()));
		ASSERT_THAT(AreEqual(TestId, EntityWithId.GetFlecsId()));
		
		ASSERT_THAT(IsTrue(World()->IsAlive(EntityWithId)));
		
		ASSERT_THAT(AreEqual(EntityWithId, World()->GetAlive(TestId)));
	}

	TEST_METHOD(CanAddRemoveTag)
	{
		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		
		const FFlecsEntityHandle Tag = World()->CreateEntity();
		ASSERT_THAT(IsTrue(Tag.IsValid()));
		
		TestEntity.Add(Tag);
		ASSERT_THAT(IsTrue(TestEntity.Has(Tag)));
		
		TestEntity.Remove(Tag);
		ASSERT_THAT(IsFalse(TestEntity.Has(Tag)));
	}

	TEST_METHOD(CanCreateNamedEntity)
	{
		static FString EntityName = TEXT("MyTestEntity");
		
		const FFlecsEntityHandle NamedEntity = World()->CreateEntity(EntityName);
		ASSERT_THAT(IsTrue(NamedEntity.IsValid()));
		ASSERT_THAT(IsTrue(NamedEntity.HasName()));
		ASSERT_THAT(AreEqual(EntityName, NamedEntity.GetName()));

		ASSERT_THAT(AreEqual(
			NamedEntity,
			World()->LookupEntity(EntityName)
		));
	}

	TEST_METHOD(CanSetThenClearEntityName)
	{
		static FString NewEntityName = TEXT("MyRenamedTestEntity");

		const FFlecsEntityHandle TestEntity = World()->CreateEntity();
		ASSERT_THAT(IsTrue(TestEntity.IsValid()));
		ASSERT_THAT(IsFalse(TestEntity.HasName()));
		ASSERT_THAT(IsFalse(World()->LookupEntity(NewEntityName).IsValid()));
		
		TestEntity.SetName(NewEntityName);
		ASSERT_THAT(AreEqual(NewEntityName, TestEntity.GetName()));
		
		ASSERT_THAT(AreEqual(
			TestEntity,
			World()->LookupEntity(NewEntityName)
		));
		
		TestEntity.ClearName();
		ASSERT_THAT(IsFalse(TestEntity.HasName()));

		ASSERT_THAT(IsFalse(World()->LookupEntity(NewEntityName).IsValid()));
	}
	
}; // UnrealFlecsBasicTests

#endif // #if WITH_AUTOMATION_TESTS
