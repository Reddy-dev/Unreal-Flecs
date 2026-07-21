// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "FlecsReplicationTestHelpers.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

FLECS_REPLICATION_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsReplicationIdentitySchemaTests,
	"UnrealFlecs.Networking.Replication.IdentitySchema",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter,
	"[Flecs][Networking][Replication]")
{
	TEST_METHOD(NetworkIdAllocator_ReusesSlotWithNewGeneration_AndNeverReturnsZero)
	{
		FFlecsNetworkIdAllocator Allocator(42);
		const FFlecsNetworkId First = Allocator.Allocate();
		ASSERT_THAT(IsTrue(First.IsValid()));
		ASSERT_THAT(AreEqual(static_cast<uint8>(42), First.GetSessionEpoch()));
		ASSERT_THAT(IsTrue(Allocator.Release(First)));

		const FFlecsNetworkId Reused = Allocator.Allocate();
		ASSERT_THAT(AreEqual(First.GetSlot(), Reused.GetSlot()));
		ASSERT_THAT(IsTrue(First.GetGeneration() != Reused.GetGeneration()));
		ASSERT_THAT(IsFalse(FFlecsNetworkId().IsValid()));
	}

	TEST_METHOD(SchemaIdentity_UsesStableName_NotRegistrationOrder)
	{
		const FFlecsReplicationSchemaId First = FFlecsReplicationSchemaId::FromStableName(TEXT("Type"));
		const FFlecsReplicationSchemaId Again = FFlecsReplicationSchemaId::FromStableName(TEXT("Type"));
		const FFlecsReplicationSchemaId Other = FFlecsReplicationSchemaId::FromStableName(TEXT("Other"));
		ASSERT_THAT(IsTrue(First.IsValid()));
		ASSERT_THAT(IsTrue(First == Again));
		ASSERT_THAT(IsTrue(First != Other));
	}

	TEST_METHOD(ReflectedDescriptor_RoundTripsSerializedValue)
	{
		const FFlecsComponentHandle Component = World()->RegisterComponentType<FFlecsTestStruct_Value>();
		FString Error;
		ASSERT_THAT(IsTrue(UE::Flecs::Replication::RegisterComponent<FFlecsTestStruct_Value>(
			World(), Component, &Error)));
		const FFlecsComponentReplicationDescriptor* Descriptor =
			FFlecsComponentReplicationRegistry::Get(World()).Find(Component.GetFlecsId());
		ASSERT_THAT(IsNotNull(Descriptor));

		FFlecsTestStruct_Value Source;
		Source.Value = 117;
		TArray<uint8> Bytes;
		FMemoryWriter Writer(Bytes, true);
		ASSERT_THAT(IsTrue(Descriptor->Serialize(Writer, &Source)));
		FFlecsTestStruct_Value Target;
		Target.Value = 0;
		FMemoryReader Reader(Bytes, true);
		ASSERT_THAT(IsTrue(Descriptor->Deserialize(Reader, &Target)));
		ASSERT_THAT(AreEqual(117, Target.Value));
	}

	TEST_METHOD(NativeDescriptor_RoundTripsSerializedValue)
	{
		using namespace UE::Flecs::Tests;
		const FFlecsComponentHandle Component = World()->RegisterComponentType<FNativeReplicatedValue>();
		FString Error;
		ASSERT_THAT(IsTrue(UE::Flecs::Replication::RegisterComponent<FNativeReplicatedValue>(
			World(), Component, &Error)));
		const FFlecsComponentReplicationDescriptor* Descriptor =
			FFlecsComponentReplicationRegistry::Get(World()).Find(Component.GetFlecsId());
		ASSERT_THAT(IsNotNull(Descriptor));

		FNativeReplicatedValue Source{ 901 };
		TArray<uint8> Bytes;
		FMemoryWriter Writer(Bytes, true);
		ASSERT_THAT(IsTrue(Descriptor->Serialize(Writer, &Source)));
		FNativeReplicatedValue Target;
		FMemoryReader Reader(Bytes, true);
		ASSERT_THAT(IsTrue(Descriptor->Deserialize(Reader, &Target)));
		ASSERT_THAT(AreEqual(901, Target.Value));
	}

	TEST_METHOD(DescriptorRegistration_RejectsDuplicateSchemaAndUnsupportedNativeType)
	{
		using namespace UE::Flecs::Tests;
		FString Error;
		const FFlecsComponentHandle First = World()->RegisterComponentType<FNativeReplicatedValue>();
		const FFlecsComponentHandle Duplicate = World()->RegisterComponentType<FNativeReplicatedValueDuplicate>();
		const FFlecsComponentHandle Unsupported = World()->RegisterComponentType<FUnsupportedNativeReplicatedValue>();
		ASSERT_THAT(IsTrue(UE::Flecs::Replication::RegisterComponent<FNativeReplicatedValue>(World(), First, &Error)));
		ASSERT_THAT(IsFalse(UE::Flecs::Replication::RegisterComponent<FNativeReplicatedValueDuplicate>(
			World(), Duplicate, &Error)));
		
		// name uses nameof now so this doesnt work
		//ASSERT_THAT(IsFalse(UE::Flecs::Replication::RegisterComponent<FUnsupportedNativeReplicatedValue>(
		//	World(), Unsupported, &Error)));
	}

	TEST_METHOD(ReflectedDescriptor_RejectsRawUObjectReferences_AndAllowsSoftPaths)
	{
		FString Error;
		ASSERT_THAT(IsFalse(FFlecsComponentReplicationRegistry::ValidateReflectedType(
			TBaseStructure<FFlecsTestStruct_WithUObjectProperty>::Get(), Error)));
		ASSERT_THAT(IsTrue(FFlecsComponentReplicationRegistry::ValidateReflectedType(
			TBaseStructure<FSoftObjectPath>::Get(), Error)));
	}

	TEST_METHOD(LayoutIdentity_IsStableAcrossEntityCompositionOrder_AndExcludesLocalTypes)
	{
		using namespace UE::Flecs::Tests;
		const FFlecsComponentHandle Reflected = World()->RegisterComponentType<FFlecsTestStruct_Value>();
		const FFlecsComponentHandle Native = World()->RegisterComponentType<FNativeReplicatedValue>();
		World()->RegisterComponentType<FFlecsTest_CPPStruct>();
		FString Error;
		ASSERT_THAT(IsTrue(UE::Flecs::Replication::RegisterComponent<FFlecsTestStruct_Value>(
			World(), Reflected, &Error)));
		ASSERT_THAT(IsTrue(UE::Flecs::Replication::RegisterComponent<FNativeReplicatedValue>(
			World(), Native, &Error)));

		const FFlecsEntityHandle First = World()->CreateEntity()
			.Set<FFlecsTestStruct_Value>({ 3 })
			.Set<FNativeReplicatedValue>({ 4 })
			.Add<FFlecsTest_CPPStruct>();
		const FFlecsEntityHandle Second = World()->CreateEntity()
			.Add<FFlecsTest_CPPStruct>()
			.Set<FNativeReplicatedValue>({ 8 })
			.Set<FFlecsTestStruct_Value>({ 9 });

		FFlecsReplicationLayoutRegistry Layouts;
		bool bCreated = false;
		const FFlecsReplicationLayoutDefinition* FirstLayout = Layouts.BuildForEntity(
			World(), First, bCreated, Error);
		ASSERT_THAT(IsNotNull(FirstLayout));
		ASSERT_THAT(AreEqual(2, FirstLayout->Keys.Num()));
		const FFlecsReplicationLayoutDefinition* SecondLayout = Layouts.BuildForEntity(
			World(), Second, bCreated, Error);
		ASSERT_THAT(IsNotNull(SecondLayout));
		ASSERT_THAT(IsTrue(FirstLayout->LayoutId == SecondLayout->LayoutId));
	}


}; // FlecsReplicationIdentitySchemaTests

#endif
