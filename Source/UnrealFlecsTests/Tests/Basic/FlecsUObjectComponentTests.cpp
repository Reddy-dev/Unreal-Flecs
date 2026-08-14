
#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"
// Elie Wiese-Namir © 2025. All Rights Reserved.

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Components/FlecsUObjectComponent.h"
#include "Components/FlecsAddReferencedObjectsTrait.h"
#include "Components/ObjectTypes/FFlecsUObjectTag.h"
#include "Worlds/FlecsWorld.h"

FLECS_TEST_CLASS_WITH_FLAGS(FlecsUObjectComponentTests,
								   "UnrealFlecs.Components.UObject",
							   EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
							    | EAutomationTestFlags::CriticalPriority)
{
	TEST_METHOD(DefaultConstruction_IsInvalid)
	{
		const FFlecsUObjectComponent Component;

		ASSERT_THAT(IsFalse(Component.IsValid()));
		ASSERT_THAT(IsFalse(static_cast<bool>(Component)));
		ASSERT_THAT(IsTrue(Component.GetObject() == nullptr));
	}

	TEST_METHOD(ConstructionWithValidUObject_IsValid)
	{
		UFlecsUObjectComponentTestObject* TestObject
			= NewObject<UFlecsUObjectComponentTestObject>(GetTransientPackage());
		const FFlecsUObjectComponent Component(TestObject);

		ASSERT_THAT(IsTrue(Component.IsValid()));
		ASSERT_THAT(IsTrue(static_cast<bool>(Component)));
		ASSERT_THAT(IsTrue(Component.GetObject() == TestObject));
	}

	TEST_METHOD(SetObject_UpdatesStoredObject)
	{
		UFlecsUObjectComponentTestObject* TestObject
			= NewObject<UFlecsUObjectComponentTestObject>(GetTransientPackage());
		FFlecsUObjectComponent Component;

		ASSERT_THAT(IsFalse(Component.IsValid()));

		Component.SetObject(TestObject);

		ASSERT_THAT(IsTrue(Component.IsValid()));
		ASSERT_THAT(IsTrue(Component.GetObject() == TestObject));
	}

	TEST_METHOD(Reset_ClearsStoredObject)
	{
		UFlecsUObjectComponentTestObject* TestObject
			= NewObject<UFlecsUObjectComponentTestObject>(GetTransientPackage());
		FFlecsUObjectComponent Component(TestObject);

		ASSERT_THAT(IsTrue(Component.IsValid()));

		Component.Reset();

		ASSERT_THAT(IsFalse(Component.IsValid()));
		ASSERT_THAT(IsTrue(Component.GetObject() == nullptr));
	}

	TEST_METHOD(EqualityOperators)
	{
		UFlecsUObjectComponentTestObject* ObjectA
			= NewObject<UFlecsUObjectComponentTestObject>(GetTransientPackage());
		UFlecsUObjectComponentTestObject* ObjectB
			= NewObject<UFlecsUObjectComponentTestObject>(GetTransientPackage());

		const FFlecsUObjectComponent ComponentA(ObjectA);
		const FFlecsUObjectComponent ComponentA2(ObjectA);
		const FFlecsUObjectComponent ComponentB(ObjectB);
		const FFlecsUObjectComponent ComponentEmpty;

		ASSERT_THAT(IsTrue(ComponentA == ComponentA2));
		ASSERT_THAT(IsFalse(ComponentA != ComponentA2));
		ASSERT_THAT(IsTrue(ComponentA != ComponentB));
		ASSERT_THAT(IsFalse(ComponentA == ComponentB));
		ASSERT_THAT(IsTrue(ComponentA != ComponentEmpty));
		ASSERT_THAT(IsFalse(ComponentA == ComponentEmpty));
	}

	TEST_METHOD(TypedGetObject_ReturnsCorrectCast)
	{
		UFlecsUObjectComponentTestObject* TestObject
			= NewObject<UFlecsUObjectComponentTestObject>(GetTransientPackage());
		const FFlecsUObjectComponent Component(TestObject);

		UFlecsUObjectComponentTestObject* Retrieved
			= Component.GetObject<UFlecsUObjectComponentTestObject>();

		ASSERT_THAT(IsTrue(Retrieved == TestObject));
		ASSERT_THAT(IsTrue(Component.GetObject<UActorComponent>() == nullptr));
	}

	TEST_METHOD(IsA_ReturnsCorrectResult)
	{
		UFlecsUObjectComponentTestObject* TestObject
			= NewObject<UFlecsUObjectComponentTestObject>(GetTransientPackage());
		const FFlecsUObjectComponent Component(TestObject);

		ASSERT_THAT(IsTrue(Component.IsA<UFlecsUObjectComponentTestObject>()));
		ASSERT_THAT(IsTrue(Component.IsA<UObject>()));
		ASSERT_THAT(IsFalse(Component.IsA<AActor>()));
	}

	TEST_METHOD(ToString_ReturnsObjectNameOrInvalid)
	{
		UFlecsUObjectComponentTestObject* TestObject
			= NewObject<UFlecsUObjectComponentTestObject>(GetTransientPackage());
		const FFlecsUObjectComponent ComponentValid(TestObject);
		const FFlecsUObjectComponent ComponentInvalid;

		ASSERT_THAT(IsTrue(ComponentValid.ToString() == TestObject->GetName()));
		ASSERT_THAT(IsTrue(ComponentInvalid.ToString() == TEXT("Invalid")));
	}

	TEST_METHOD(SetPair_HasPairReturnsTrue)
	{
		UFlecsUObjectComponentTestObject* TestObject
			= NewObject<UFlecsUObjectComponentTestObject>(GetTransientPackage());
		const FFlecsEntityHandle Entity = World()->CreateEntity();

		Entity.SetPair<FFlecsUObjectComponent, FFlecsUObjectTag>(FFlecsUObjectComponent{ TestObject });

		ASSERT_THAT(IsTrue(Entity.HasPair<FFlecsUObjectComponent, FFlecsUObjectTag>()));
	}

	TEST_METHOD(SetPair_GetPairFirstReturnsCorrectComponent)
	{
		UFlecsUObjectComponentTestObject* TestObject
			= NewObject<UFlecsUObjectComponentTestObject>(GetTransientPackage());
		const FFlecsEntityHandle Entity = World()->CreateEntity();

		Entity.SetPair<FFlecsUObjectComponent, FFlecsUObjectTag>(FFlecsUObjectComponent{ TestObject });

		const FFlecsUObjectComponent& Component
			= Entity.GetPairFirst<FFlecsUObjectComponent, FFlecsUObjectTag>();

		ASSERT_THAT(IsTrue(Component.IsValid()));
		ASSERT_THAT(IsTrue(Component.GetObject() == TestObject));
	}

	TEST_METHOD(RemovePair_HasPairReturnsFalse)
	{
		UFlecsUObjectComponentTestObject* TestObject
			= NewObject<UFlecsUObjectComponentTestObject>(GetTransientPackage());
		const FFlecsEntityHandle Entity = World()->CreateEntity();

		Entity.SetPair<FFlecsUObjectComponent, FFlecsUObjectTag>(FFlecsUObjectComponent{ TestObject });
		ASSERT_THAT(IsTrue(Entity.HasPair<FFlecsUObjectComponent, FFlecsUObjectTag>()));

		Entity.RemovePair<FFlecsUObjectComponent, FFlecsUObjectTag>();
		ASSERT_THAT(IsFalse(Entity.HasPair<FFlecsUObjectComponent, FFlecsUObjectTag>()));
	}

	TEST_METHOD(MultipleEntities_HaveIndependentComponents)
	{
		UFlecsUObjectComponentTestObject* ObjA
			= NewObject<UFlecsUObjectComponentTestObject>(GetTransientPackage());
		UFlecsUObjectComponentTestObject* ObjB
			= NewObject<UFlecsUObjectComponentTestObject>(GetTransientPackage());

		const FFlecsEntityHandle EntityA = World()->CreateEntity();
		const FFlecsEntityHandle EntityB = World()->CreateEntity();

		EntityA.SetPair<FFlecsUObjectComponent, FFlecsUObjectTag>(FFlecsUObjectComponent{ ObjA });
		EntityB.SetPair<FFlecsUObjectComponent, FFlecsUObjectTag>(FFlecsUObjectComponent{ ObjB });

		const UObject* RetrievedA
			= EntityA.GetPairFirst<FFlecsUObjectComponent, FFlecsUObjectTag>().GetObject();
		const UObject* RetrievedB
			= EntityB.GetPairFirst<FFlecsUObjectComponent, FFlecsUObjectTag>().GetObject();

		ASSERT_THAT(IsTrue(RetrievedA == ObjA));
		ASSERT_THAT(IsTrue(RetrievedB == ObjB));
		ASSERT_THAT(IsTrue(RetrievedA != RetrievedB));
	}

	TEST_METHOD(ComponentEntity_HasAddReferencedObjectsTrait)
	{
		const TFlecsComponentHandle<FFlecsTestStruct_WithUObjectProperty> ComponentHandle
			= World()->RegisterComponentType<FFlecsTestStruct_WithUObjectProperty>();

		ASSERT_THAT(IsTrue(ComponentHandle.IsValid()));
		ASSERT_THAT(IsTrue(ComponentHandle.Has<FFlecsAddReferencedObjectsTrait>()));
	}

}; // TEST_CLASS FlecsUObjectComponentTests

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS
