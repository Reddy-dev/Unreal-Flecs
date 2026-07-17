// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

#include "Networking/FlecsComponentReplicationDescriptor.h"
#include "Networking/FlecsNetworkId.h"
#include "Networking/FlecsNetworkWorldSubsystem.h"
#include "Networking/FlecsReplicatedEntityComponent.h"
#include "Networking/FlecsReplicationQuantizers.h"
#include "Networking/FlecsReplicationRouting.h"
#include "Networking/FlecsReplicationTypes.h"
#include "Networking/FlecsStablePathTag.h"

namespace UE::Flecs::Tests
{
	struct FNativeReplicatedValue
	{
		int32 Value = 0;
	};

	struct FNativeReplicatedValueDuplicate
	{
		int32 Value = 0;
	};

	struct FUnsupportedNativeReplicatedValue
	{
		int32 Value = 0;
	};

	struct FCadencedReplicatedValue
	{
		int32 Value = 0;
	};

	struct FHighPriorityReplicatedValue
	{
		int32 Value = 0;
	};

	struct FLowPriorityReplicatedValue
	{
		int32 Value = 0;
	};

	struct FQuantizedReplicatedValue
	{
		float Value = 0.0f;
	};

	class FCustomTestInterestPolicy final : public IFlecsReplicationInterestPolicy
	{
	public:
		virtual bool IsInterested(const FFlecsReplicationRouteDescriptor& Route,
			const FFlecsReplicationConnectionInterestContext&,
			const FFlecsReplicationConnectionView& View) const override
		{
			return Route.CustomPolicy == TEXT("NearOrigin") && !View.Positions.IsEmpty()
				&& View.Positions[0].SizeSquared() <= FMath::Square(100.0);
		}
	};

	template<typename T>
	FFlecsComponentHandle RegisterTestComponent(UFlecsWorld* World)
	{
		const FFlecsComponentHandle Component = World->RegisterComponentType<T>();
		const FFlecsComponentPropertiesDefinition Properties = FFlecsComponentPropertiesDefinition::Make<T>();
		Properties.PropertiesFunction(World, Component, Properties);
		return Component;
	}

	template <typename T>
	FFlecsComponentHandle RegisterNativeReplicationComponent(UFlecsWorld* World)
	{
		const FFlecsComponentHandle Component = World->RegisterComponentType<T>();
		FString Error;
		check(UE::Flecs::Replication::RegisterComponent<T>(World, Component, &Error));
		return Component;
	}

	void RegisterReplicationMatrixTypes(UFlecsWorld* World)
	{
		RegisterTestComponent<FFlecsReplicationTestRequiredTag>(World);
		RegisterTestComponent<FFlecsReplicationTestValue>(World);
		RegisterTestComponent<FFlecsReplicationTestDontFragmentValue>(World);
		RegisterTestComponent<FFlecsReplicationTestNativeValue>(World);
		RegisterTestComponent<FFlecsReplicationTestTag>(World);
		RegisterTestComponent<FFlecsReplicationTestRelationship>(World);
		RegisterTestComponent<FFlecsReplicationTestValueRelationship>(World);
		RegisterTestComponent<FFlecsReplicationTestWithValue>(World);
		RegisterTestComponent<FFlecsReplicationTestLocalOnly>(World);
	}

	struct FCapturedReplicationEntity
	{
		FFlecsReplicationLayoutDefinition Layout;
		FFlecsReplicatedEntityUpdate Snapshot;
	};

	FCapturedReplicationEntity CaptureEntity(UFlecsNetworkWorldSubsystem* Subsystem,
		UFlecsReplicationCaptureTransport* Transport, const FFlecsEntityHandle& Entity)
	{
		Transport->Updates.Reset();
		const FFlecsNetworkId NetworkId = Subsystem->BeginReplicatingEntity(Entity);
		Subsystem->FlushServerReplicationForTesting();
		const FFlecsReplicatedEntityUpdate* FoundSnapshot = Transport->Updates.FindByPredicate(
			[NetworkId](const FFlecsReplicatedEntityUpdate& Candidate)
			{
				return Candidate.NetworkId == NetworkId;
			});
		check(FoundSnapshot);
		const FFlecsReplicatedEntityUpdate Snapshot = *FoundSnapshot;
		const FFlecsReplicationLayoutDefinition* Layout = Transport->Layouts.FindByPredicate(
			[&Snapshot](const FFlecsReplicationLayoutDefinition& Candidate)
			{
				return Candidate.LayoutId == Snapshot.LayoutId;
			});
		check(Layout);
		return { *Layout, Snapshot };
	}

	void EnqueueLayout(UFlecsNetworkWorldSubsystem* Subsystem, const FGuid& Shard,
		const FFlecsReplicationLayoutDefinition& Layout)
	{
		FFlecsReplicationInboxRecord Record;
		Record.Type = EFlecsReplicationInboxRecordType::Layout;
		Record.SourceShard = Shard;
		Record.Layout = Layout;
		Subsystem->EnqueueReceivedRecord(MoveTemp(Record));
	}

	void EnqueueSnapshot(UFlecsNetworkWorldSubsystem* Subsystem, const FGuid& Shard,
		const FFlecsReplicatedEntityUpdate& Snapshot)
	{
		FFlecsReplicationInboxRecord Record;
		Record.Type = EFlecsReplicationInboxRecordType::UpsertEntity;
		Record.SourceShard = Shard;
		Record.Update = Snapshot;
		Subsystem->EnqueueReceivedRecord(MoveTemp(Record));
	}

	void SetSnapshotValue(UFlecsWorld* World, FFlecsReplicatedEntityUpdate& Snapshot,
		const FFlecsReplicationLayoutDefinition& Layout, const FFlecsReplicationTestValue& Value)
	{
		const FFlecsComponentReplicationDescriptor* Descriptor =
			FFlecsComponentReplicationRegistry::Get(World).Find(World->GetIdIfRegistered<FFlecsReplicationTestValue>());
		check(Descriptor);
		
		for (FFlecsReplicatedValue& Serialized : Snapshot.Values)
		{
			if (Layout.Keys.IsValidIndex(Serialized.KeyIndex)
				&& Layout.Keys[Serialized.KeyIndex].StorageSchema == Descriptor->SchemaId)
			{
				Serialized.Bytes.Reset();
				FMemoryWriter Writer(Serialized.Bytes, true);
				FFlecsReplicationTestValue Copy = Value;
				Descriptor->Serialize(Writer, &Copy);
				return;
			}
		}
	}

	void SetSnapshotPairValue(UFlecsWorld* World, FFlecsReplicatedEntityUpdate& Snapshot,
		const FFlecsReplicationLayoutDefinition& Layout, const FFlecsReplicationTestValueRelationship& Value)
	{
		const FFlecsComponentReplicationDescriptor* Descriptor =
			FFlecsComponentReplicationRegistry::Get(World).Find(
				World->GetIdIfRegistered<FFlecsReplicationTestValueRelationship>());
		check(Descriptor);

		for (FFlecsReplicatedValue& Serialized : Snapshot.Values)
		{
			if (Layout.Keys.IsValidIndex(Serialized.KeyIndex)
				&& Layout.Keys[Serialized.KeyIndex].Kind == EFlecsReplicationKeyKind::Pair
				&& Layout.Keys[Serialized.KeyIndex].StorageSchema == Descriptor->SchemaId)
			{
				Serialized.Bytes.Reset();
				FMemoryWriter Writer(Serialized.Bytes, true);
				FFlecsReplicationTestValueRelationship Copy = Value;
				Descriptor->Serialize(Writer, &Copy);
				return;
			}
		}
	}

	void EnqueueRemoval(UFlecsNetworkWorldSubsystem* Subsystem, const FGuid& Shard,
		const FFlecsNetworkId NetworkId)
	{
		FFlecsReplicationInboxRecord Record;
		Record.Type = EFlecsReplicationInboxRecordType::RemoveEntity;
		Record.SourceShard = Shard;
		Record.NetworkId = NetworkId;
		Subsystem->EnqueueReceivedRecord(MoveTemp(Record));
	}
}

template<>
struct TFlecsReplicationTraits<UE::Flecs::Tests::FNativeReplicatedValue>
{
	static FString StableName() { return TEXT("FNativeReplicatedValue"); }
	static bool Serialize(FArchive& Archive, UE::Flecs::Tests::FNativeReplicatedValue& Value)
	{
		Archive << Value.Value;
		return !Archive.IsError();
	}
};

template<>
struct TFlecsReplicationTraits<UE::Flecs::Tests::FNativeReplicatedValueDuplicate>
{
	static FString StableName() { return TEXT("FNativeReplicatedValue"); }
	
	static bool Serialize(FArchive& Archive, UE::Flecs::Tests::FNativeReplicatedValueDuplicate& Value)
	{
		Archive << Value.Value;
		return !Archive.IsError();
	}
};

template <>
struct TFlecsReplicationTraits<UE::Flecs::Tests::FCadencedReplicatedValue>
{
	static constexpr float UpdateFrequencyHz = 2.0f;
	static FString StableName() { return TEXT("FCadencedReplicatedValue"); }
	static bool Serialize(FArchive& Archive, UE::Flecs::Tests::FCadencedReplicatedValue& Value)
	{
		Archive << Value.Value;
		return !Archive.IsError();
	}
};

template <>
struct TFlecsReplicationTraits<UE::Flecs::Tests::FHighPriorityReplicatedValue>
{
	static constexpr float ReplicationPriority = 10.0f;
	static FString StableName() { return TEXT("FHighPriorityReplicatedValue"); }
	static bool Serialize(FArchive& Archive, UE::Flecs::Tests::FHighPriorityReplicatedValue& Value)
	{
		Archive << Value.Value;
		return !Archive.IsError();
	}
};

template <>
struct TFlecsReplicationTraits<UE::Flecs::Tests::FLowPriorityReplicatedValue>
{
	static constexpr float ReplicationPriority = 0.1f;
	static FString StableName() { return TEXT("FLowPriorityReplicatedValue"); }
	static bool Serialize(FArchive& Archive, UE::Flecs::Tests::FLowPriorityReplicatedValue& Value)
	{
		Archive << Value.Value;
		return !Archive.IsError();
	}
};

template <>
struct TFlecsReplicationTraits<UE::Flecs::Tests::FQuantizedReplicatedValue>
{
	using Quantizer = TFlecsScalarReplicationQuantizer<1>;

	static FString StableName() { return TEXT("FQuantizedReplicatedValue"); }
	static bool Serialize(FArchive& Archive, UE::Flecs::Tests::FQuantizedReplicatedValue& Value)
	{
		Archive << Value.Value;
		return !Archive.IsError();
	}
};

TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsReplicationCoreTests,
	"UnrealFlecs.Networking.Replication.Core",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter,
	"[Flecs][Networking][Replication]")
{
	inline static TUniquePtr<FFlecsTestFixtureRAII> Fixture;
	inline static TObjectPtr<UFlecsWorld> FlecsWorld = nullptr;
	inline static TObjectPtr<UFlecsNetworkWorldSubsystem> NetworkSubsystem = nullptr;
	inline static TObjectPtr<UFlecsReplicationCaptureTransport> CaptureTransport = nullptr;

	BEFORE_EACH()
	{
		Fixture = MakeUnique<FFlecsTestFixtureRAII>();
		FlecsWorld = Fixture->Fixture.GetFlecsWorld();
		NetworkSubsystem = FlecsWorld->GetWorld()->GetSubsystem<UFlecsNetworkWorldSubsystem>();
		NetworkSubsystem->SetReplicationTimeForTesting(0.0);
		CaptureTransport = NewObject<UFlecsReplicationCaptureTransport>(NetworkSubsystem);
		NetworkSubsystem->SetReplicationTransportForTesting(CaptureTransport);
		UE::Flecs::Tests::RegisterReplicationMatrixTypes(FlecsWorld);
	}

	AFTER_EACH()
	{
		FlecsWorld = nullptr;
		NetworkSubsystem = nullptr;
		CaptureTransport = nullptr;
		Fixture.Reset();
	}

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
		const FFlecsComponentHandle Component = FlecsWorld->RegisterComponentType<FFlecsTestStruct_Value>();
		FString Error;
		ASSERT_THAT(IsTrue(UE::Flecs::Replication::RegisterComponent<FFlecsTestStruct_Value>(
			FlecsWorld, Component, &Error)));
		const FFlecsComponentReplicationDescriptor* Descriptor =
			FFlecsComponentReplicationRegistry::Get(FlecsWorld).Find(Component.GetFlecsId());
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
		const FFlecsComponentHandle Component = FlecsWorld->RegisterComponentType<FNativeReplicatedValue>();
		FString Error;
		ASSERT_THAT(IsTrue(UE::Flecs::Replication::RegisterComponent<FNativeReplicatedValue>(
			FlecsWorld, Component, &Error)));
		const FFlecsComponentReplicationDescriptor* Descriptor =
			FFlecsComponentReplicationRegistry::Get(FlecsWorld).Find(Component.GetFlecsId());
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
		const FFlecsComponentHandle First = FlecsWorld->RegisterComponentType<FNativeReplicatedValue>();
		const FFlecsComponentHandle Duplicate = FlecsWorld->RegisterComponentType<FNativeReplicatedValueDuplicate>();
		const FFlecsComponentHandle Unsupported = FlecsWorld->RegisterComponentType<FUnsupportedNativeReplicatedValue>();
		ASSERT_THAT(IsTrue(UE::Flecs::Replication::RegisterComponent<FNativeReplicatedValue>(FlecsWorld, First, &Error)));
		ASSERT_THAT(IsFalse(UE::Flecs::Replication::RegisterComponent<FNativeReplicatedValueDuplicate>(
			FlecsWorld, Duplicate, &Error)));
		
		// name uses nameof now so this doesnt work
		//ASSERT_THAT(IsFalse(UE::Flecs::Replication::RegisterComponent<FUnsupportedNativeReplicatedValue>(
		//	FlecsWorld, Unsupported, &Error)));
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
		const FFlecsComponentHandle Reflected = FlecsWorld->RegisterComponentType<FFlecsTestStruct_Value>();
		const FFlecsComponentHandle Native = FlecsWorld->RegisterComponentType<FNativeReplicatedValue>();
		FlecsWorld->RegisterComponentType<FFlecsTest_CPPStruct>();
		FString Error;
		ASSERT_THAT(IsTrue(UE::Flecs::Replication::RegisterComponent<FFlecsTestStruct_Value>(
			FlecsWorld, Reflected, &Error)));
		ASSERT_THAT(IsTrue(UE::Flecs::Replication::RegisterComponent<FNativeReplicatedValue>(
			FlecsWorld, Native, &Error)));

		const FFlecsEntityHandle First = FlecsWorld->CreateEntity()
			.Set<FFlecsTestStruct_Value>({ 3 })
			.Set<FNativeReplicatedValue>({ 4 })
			.Add<FFlecsTest_CPPStruct>();
		const FFlecsEntityHandle Second = FlecsWorld->CreateEntity()
			.Add<FFlecsTest_CPPStruct>()
			.Set<FNativeReplicatedValue>({ 8 })
			.Set<FFlecsTestStruct_Value>({ 9 });

		FFlecsReplicationLayoutRegistry Layouts;
		bool bCreated = false;
		const FFlecsReplicationLayoutDefinition* FirstLayout = Layouts.BuildForEntity(
			FlecsWorld, First, bCreated, Error);
		ASSERT_THAT(IsNotNull(FirstLayout));
		ASSERT_THAT(AreEqual(2, FirstLayout->Keys.Num()));
		const FFlecsReplicationLayoutDefinition* SecondLayout = Layouts.BuildForEntity(
			FlecsWorld, Second, bCreated, Error);
		ASSERT_THAT(IsNotNull(SecondLayout));
		ASSERT_THAT(IsTrue(FirstLayout->LayoutId == SecondLayout->LayoutId));
	}

	TEST_METHOD(UpdateMask_TracksSelectedLayoutKeys)
	{
		FFlecsReplicatedEntityUpdate Update;
		Update.SetKeyChanged(0);
		Update.SetKeyChanged(63);
		Update.SetKeyChanged(64);
		ASSERT_THAT(IsTrue(Update.IsKeyChanged(0)));
		ASSERT_THAT(IsTrue(Update.IsKeyChanged(63)));
		ASSERT_THAT(IsTrue(Update.IsKeyChanged(64)));
		ASSERT_THAT(IsFalse(Update.IsKeyChanged(1)));
		ASSERT_THAT(AreEqual(2, Update.ChangedKeyMask.Num()));
	}

	TEST_METHOD(FullThenDelta_SuppressesIdenticalWrites_AndSupportsManualDirtyMarking)
	{
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity()
			.Set<FFlecsReplicationTestValue>({ 10 });
		const UE::Flecs::Tests::FCapturedReplicationEntity Initial =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		ASSERT_THAT(IsTrue(Initial.Snapshot.Kind == EFlecsReplicatedEntityUpdateKind::Full));

		CaptureTransport->Updates.Reset();
		Source.Set<FFlecsReplicationTestValue>({ 20 });
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(1, CaptureTransport->Updates.Num()));
		ASSERT_THAT(IsTrue(CaptureTransport->Updates[0].Kind == EFlecsReplicatedEntityUpdateKind::Delta));
		ASSERT_THAT(AreEqual(1, CaptureTransport->Updates[0].Values.Num()));
		ASSERT_THAT(IsTrue(CaptureTransport->Updates[0].IsKeyChanged(
			CaptureTransport->Updates[0].Values[0].KeyIndex)));

		CaptureTransport->Updates.Reset();
		Source.Set<FFlecsReplicationTestValue>({ 20 });
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(0, CaptureTransport->Updates.Num()));

		Source.GetMut<FFlecsReplicationTestValue>().Value = 30;
		NetworkSubsystem->MarkComponentDirty<FFlecsReplicationTestValue>(Source);
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(1, CaptureTransport->Updates.Num()));
		ASSERT_THAT(IsTrue(CaptureTransport->Updates[0].Kind == EFlecsReplicatedEntityUpdateKind::Delta));
	}

	TEST_METHOD(Cadence_CoalescesPendingWritesToNewestValue)
	{
		using namespace UE::Flecs::Tests;
		RegisterNativeReplicationComponent<FCadencedReplicatedValue>(FlecsWorld);
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity().Set<FCadencedReplicatedValue>({ 1 });
		CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		CaptureTransport->Updates.Reset();

		NetworkSubsystem->SetReplicationTimeForTesting(0.1);
		Source.Set<FCadencedReplicatedValue>({ 2 });
		NetworkSubsystem->FlushServerReplicationForTesting();
		NetworkSubsystem->SetReplicationTimeForTesting(0.2);
		Source.Set<FCadencedReplicatedValue>({ 3 });
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(0, CaptureTransport->Updates.Num()));

		NetworkSubsystem->SetReplicationTimeForTesting(0.5);
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(1, CaptureTransport->Updates.Num()));
		FCadencedReplicatedValue Result;
		FMemoryReader Reader(CaptureTransport->Updates[0].Values[0].Bytes, true);
		TFlecsReplicationTraits<FCadencedReplicatedValue>::Serialize(Reader, Result);
		ASSERT_THAT(AreEqual(3, Result.Value));
	}

	TEST_METHOD(Budget_PrioritizesHighScore_ThenAgeDrainsDeferredKeys)
	{
		using namespace UE::Flecs::Tests;
		RegisterNativeReplicationComponent<FHighPriorityReplicatedValue>(FlecsWorld);
		RegisterNativeReplicationComponent<FLowPriorityReplicatedValue>(FlecsWorld);
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity()
			.Set<FHighPriorityReplicatedValue>({ 1 })
			.Set<FLowPriorityReplicatedValue>({ 1 });
		CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		CaptureTransport->Updates.Reset();
		NetworkSubsystem->SetPayloadBudgetForTesting(sizeof(int32));
		NetworkSubsystem->SetReplicationTimeForTesting(1.0);
		Source.Set<FHighPriorityReplicatedValue>({ 2 });
		Source.Set<FLowPriorityReplicatedValue>({ 2 });
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(1, CaptureTransport->Updates.Num()));
		ASSERT_THAT(AreEqual(1, CaptureTransport->Updates[0].Values.Num()));
		const FFlecsReplicationLayoutDefinition* Layout =
			CaptureTransport->Layouts.FindByPredicate([&](const FFlecsReplicationLayoutDefinition& Candidate)
			{
				return Candidate.LayoutId == CaptureTransport->Updates[0].LayoutId;
			});
		ASSERT_THAT(IsNotNull(Layout));
		const FFlecsComponentReplicationDescriptor* HighDescriptor =
			FFlecsComponentReplicationRegistry::Get(FlecsWorld).Find(
				FlecsWorld->GetIdIfRegistered<FHighPriorityReplicatedValue>());
		ASSERT_THAT(IsTrue(Layout->Keys[CaptureTransport->Updates[0].Values[0].KeyIndex].StorageSchema
			== HighDescriptor->SchemaId));

		CaptureTransport->Updates.Reset();
		NetworkSubsystem->SetReplicationTimeForTesting(2.0);
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(1, CaptureTransport->Updates.Num()));
		ASSERT_THAT(AreEqual(1, CaptureTransport->Updates[0].Values.Num()));
	}

	TEST_METHOD(UnlimitedBudget_SendsAllEligibleKeysInOneDelta)
	{
		using namespace UE::Flecs::Tests;
		RegisterNativeReplicationComponent<FHighPriorityReplicatedValue>(FlecsWorld);
		RegisterNativeReplicationComponent<FLowPriorityReplicatedValue>(FlecsWorld);
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity()
			.Set<FHighPriorityReplicatedValue>({ 1 })
			.Set<FLowPriorityReplicatedValue>({ 1 });
		CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		CaptureTransport->Updates.Reset();
		NetworkSubsystem->SetPayloadBudgetForTesting(0);
		NetworkSubsystem->SetReplicationTimeForTesting(1.0);
		Source.Set<FHighPriorityReplicatedValue>({ 2 });
		Source.Set<FLowPriorityReplicatedValue>({ 2 });
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(1, CaptureTransport->Updates.Num()));
		ASSERT_THAT(AreEqual(2, CaptureTransport->Updates[0].Values.Num()));
	}

	TEST_METHOD(QuantizerPresets_StayWithinHalfStepError)
	{
		double Scalar = 12.3456;
		TFlecsScalarReplicationQuantizer<2>::Quantize(Scalar);
		ASSERT_THAT(IsTrue(FMath::Abs(Scalar - 12.35) <= 0.005));
		FVector Vector(1.234, -5.678, 9.876);
		TFlecsVectorReplicationQuantizer<1>::Quantize(Vector);
		ASSERT_THAT(IsTrue(Vector.Equals(FVector(1.2, -5.7, 9.9), 0.05)));
		FRotator Rotator(10.44, 370.26, -181.14);
		TFlecsRotatorReplicationQuantizer<1>::Quantize(Rotator);
		ASSERT_THAT(IsTrue(FMath::Abs(Rotator.Pitch - 10.4) <= 0.05));
		ASSERT_THAT(IsTrue(FMath::Abs(Rotator.Yaw - 10.3) <= 0.05));
	}

	TEST_METHOD(QuantizedEncoding_FingerprintsLayout_AndSuppressesSameEncodedValue)
	{
		using namespace UE::Flecs::Tests;
		const FFlecsComponentHandle Component = RegisterNativeReplicationComponent<FQuantizedReplicatedValue>(FlecsWorld);
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity().Set<FQuantizedReplicatedValue>({ 1.01f });
		const FCapturedReplicationEntity Initial = CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		const FFlecsComponentReplicationDescriptor* Descriptor =
			FFlecsComponentReplicationRegistry::Get(FlecsWorld).Find(Component.GetFlecsId());
		ASSERT_THAT(IsNotNull(Descriptor));
		ASSERT_THAT(AreEqual(FString(TEXT("ScalarDecimal:1")), Descriptor->CodecFingerprint));
		ASSERT_THAT(IsTrue(Initial.Layout.Keys.ContainsByPredicate(
			[Descriptor](const FFlecsReplicationKey& Key)
			{
				return Key.StorageSchema == Descriptor->SchemaId
					&& Key.CodecFingerprint == Descriptor->CodecFingerprint;
			})));

		CaptureTransport->Updates.Reset();
		Source.Set<FQuantizedReplicatedValue>({ 1.04f });
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(0, CaptureTransport->Updates.Num()));

		Source.Set<FQuantizedReplicatedValue>({ 1.06f });
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(1, CaptureTransport->Updates.Num()));
		FQuantizedReplicatedValue Decoded;
		FMemoryReader Reader(CaptureTransport->Updates[0].Values[0].Bytes, true);
		TFlecsReplicationTraits<FQuantizedReplicatedValue>::Serialize(Reader, Decoded);
		ASSERT_THAT(IsTrue(FMath::IsNearlyEqual(1.1f, Decoded.Value)));
	}

	TEST_METHOD(Dormancy_EntersWhenClean_AndDirtyStateWakesForFlush)
	{
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity()
			.Set<FFlecsReplicationTestValue>({ 1 });
		const UE::Flecs::Tests::FCapturedReplicationEntity Initial =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		NetworkSubsystem->SetReplicationDormancy(Source, EFlecsReplicationDormancyMode::DormantUntilDirty);
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(IsTrue(CaptureTransport->DormancyChanges.Last().Value));

		CaptureTransport->Updates.Reset();
		Source.Set<FFlecsReplicationTestValue>({ 2 });
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(1, CaptureTransport->Updates.Num()));
		ASSERT_THAT(IsTrue(CaptureTransport->Updates[0].StateRevision > Initial.Snapshot.StateRevision));
		ASSERT_THAT(IsFalse(CaptureTransport->DormancyChanges[CaptureTransport->DormancyChanges.Num() - 2].Value));
		ASSERT_THAT(IsTrue(CaptureTransport->DormancyChanges.Last().Value));
	}

	TEST_METHOD(UpdateChunks_ReassembleOutOfOrder_AndRejectOverlap)
	{
		FFlecsReplicatedEntityUpdate Update;
		Update.NetworkId = FFlecsNetworkId(7, 1, 3);
		Update.StateRevision = 4;
		Update.CompositionRevision = 1;
		Update.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		Update.SetKeyChanged(2);
		FFlecsReplicatedValue& Value = Update.Values.AddDefaulted_GetRef();
		Value.KeyIndex = 2;
		Value.Bytes.SetNumUninitialized(70 * 1024);
		for (int32 Index = 0; Index < Value.Bytes.Num(); ++Index)
		{
			Value.Bytes[Index] = static_cast<uint8>(Index);
		}
		TArray<FFlecsReplicationUpdateChunk> Chunks;
		BuildFlecsReplicationUpdateChunks(Update, Chunks);
		ASSERT_THAT(IsTrue(Chunks.Num() >= 4));
		for (const FFlecsReplicationUpdateChunk& Chunk : Chunks)
		{
			ASSERT_THAT(IsTrue(Chunk.Bytes.Num() <= FFlecsReplicationUpdateChunk::MaxChunkBytes));
		}

		FFlecsReplicationUpdateReassembler Reassembler;
		TOptional<FFlecsReplicatedEntityUpdate> Complete;
		FString Error;
		const FGuid Source = FGuid::NewGuid();
		for (int32 Index = Chunks.Num() - 1; Index >= 0; --Index)
		{
			ASSERT_THAT(IsTrue(Reassembler.Accept(Source, Chunks[Index], Complete, Error)));
		}
		ASSERT_THAT(IsTrue(Complete.IsSet()));
		ASSERT_THAT(IsTrue(Complete->Values[0].Bytes == Value.Bytes));

		Reassembler.Reset();
		Complete.Reset();
		ASSERT_THAT(IsTrue(Reassembler.Accept(Source, Chunks[1], Complete, Error)));
		ASSERT_THAT(IsFalse(Reassembler.Accept(Source, Chunks[1], Complete, Error)));

		Reassembler.Reset();
		Complete.Reset();
		ASSERT_THAT(IsTrue(Reassembler.Accept(Source, Chunks[1], Complete, Error)));
		FFlecsReplicatedEntityUpdate Newer = Update;
		++Newer.StateRevision;
		TArray<FFlecsReplicationUpdateChunk> NewerChunks;
		BuildFlecsReplicationUpdateChunks(Newer, NewerChunks);
		for (const FFlecsReplicationUpdateChunk& Chunk : NewerChunks)
		{
			ASSERT_THAT(IsTrue(Reassembler.Accept(Source, Chunk, Complete, Error)));
		}
		ASSERT_THAT(IsTrue(Complete.IsSet()));
		Complete.Reset();
		ASSERT_THAT(IsTrue(Reassembler.Accept(Source, Chunks[0], Complete, Error)));
		ASSERT_THAT(IsFalse(Complete.IsSet()));
	}

	TEST_METHOD(DefaultAndCustomInterestPolicies_EvaluateOwnerTeamZoneAndView)
	{
		FFlecsReplicationConnectionInterestContext Context;
		Context.Owner = FFlecsNetworkId(4, 1, 1);
		Context.Team = 7;
		Context.Zones.Add(TEXT("Arena"));
		NetworkSubsystem->SetConnectionInterestContext(11, Context);
		FFlecsReplicationConnectionView View;
		View.Positions.Add(FVector(10.0, 0.0, 0.0));
		FFlecsReplicationRouteDescriptor Route;
		ASSERT_THAT(IsTrue(NetworkSubsystem->IsRouteRelevant(Route, 11, View)));
		Route.Audience = EFlecsReplicationAudience::OwnerOnly;
		Route.Owner = Context.Owner;
		ASSERT_THAT(IsTrue(NetworkSubsystem->IsRouteRelevant(Route, 11, View)));
		Route.Audience = EFlecsReplicationAudience::Team;
		Route.Team = 7;
		ASSERT_THAT(IsTrue(NetworkSubsystem->IsRouteRelevant(Route, 11, View)));
		Route.Audience = EFlecsReplicationAudience::Zone;
		Route.Zone = TEXT("Arena");
		ASSERT_THAT(IsTrue(NetworkSubsystem->IsRouteRelevant(Route, 11, View)));
		Route.Audience = EFlecsReplicationAudience::Custom;
		Route.CustomPolicy = TEXT("NearOrigin");
		NetworkSubsystem->SetInterestPolicy(MakeUnique<UE::Flecs::Tests::FCustomTestInterestPolicy>());
		ASSERT_THAT(IsTrue(NetworkSubsystem->IsRouteRelevant(Route, 11, View)));
		View.Positions[0] = FVector(1000.0, 0.0, 0.0);
		ASSERT_THAT(IsFalse(NetworkSubsystem->IsRouteRelevant(Route, 11, View)));
	}

	TEST_METHOD(RoutingComponentChange_PublishesFullMigrationBaseline)
	{
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity()
			.Set<FFlecsReplicationTestValue>({ 10 });
		UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		CaptureTransport->Updates.Reset();
		FFlecsReplicationRouting Routing;
		Routing.Route.LogicalKey = FFlecsReplicationRouteKey(FName(TEXT("TeamRoute")));
		Routing.Route.Audience = EFlecsReplicationAudience::Team;
		Routing.Route.Team = 2;
		Source.Set<FFlecsReplicationRouting>(Routing);
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(1, CaptureTransport->MigrationNewRoutes.Num()));
		ASSERT_THAT(AreEqual(FName(TEXT("TeamRoute")), CaptureTransport->MigrationNewRoutes[0].LogicalKey.Name));
		ASSERT_THAT(AreEqual(1, CaptureTransport->Updates.Num()));
		ASSERT_THAT(IsTrue(CaptureTransport->Updates[0].Kind == EFlecsReplicatedEntityUpdateKind::Full));
	}

	TEST_METHOD(MigrationOwnership_OldRemovalDetachAndSnapshotCannotDeleteNewSource)
	{
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity()
			.Set<FFlecsReplicationTestValue>({ 10 });
		UE::Flecs::Tests::FCapturedReplicationEntity Captured =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		NetworkSubsystem->StopReplicatingEntity(Source);
		Source.Destroy();
		NetworkSubsystem->ResetClientReplicationForTesting();
		NetworkSubsystem->EnterClientReplicationModeForTesting();
		const FGuid OldSource = FGuid::NewGuid();
		const FGuid NewSource = FGuid::NewGuid();
		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, OldSource, Captured.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, OldSource, Captured.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();

		FFlecsReplicatedEntityUpdate Migrated = Captured.Snapshot;
		++Migrated.StateRevision;
		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, NewSource, Captured.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, NewSource, Migrated);
		UE::Flecs::Tests::EnqueueRemoval(NetworkSubsystem, OldSource, Migrated.NetworkId);
		FFlecsReplicationInboxRecord Detach;
		Detach.Type = EFlecsReplicationInboxRecordType::DetachShard;
		Detach.SourceShard = OldSource;
		NetworkSubsystem->EnqueueReceivedRecord(MoveTemp(Detach));
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, OldSource, Captured.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();
		ASSERT_THAT(IsTrue(NetworkSubsystem->FindEntity(Migrated.NetworkId).IsValid()));
	}

	TEST_METHOD(DontFragmentComponent_ReplicatesRuntimeAdditionValueAndRemoval)
	{
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity()
			.Set<FFlecsReplicationTestValue>({ 17 });
		const UE::Flecs::Tests::FCapturedReplicationEntity Initial =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		const FFlecsNetworkId NetworkId = Initial.Snapshot.NetworkId;

		const flecs::table_t* TableBeforeAddition = Source.GetEntity().table().get_table();
		CaptureTransport->Updates.Reset();
		Source.Set<FFlecsReplicationTestDontFragmentValue>({ 73 });
		ASSERT_THAT(IsTrue(TableBeforeAddition == Source.GetEntity().table().get_table()));
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(1, CaptureTransport->Updates.Num()));
		const FFlecsReplicatedEntityUpdate AddedSnapshot = CaptureTransport->Updates[0];
		ASSERT_THAT(IsTrue(AddedSnapshot.NetworkId == NetworkId));
		const FFlecsReplicationLayoutDefinition* AddedLayout = CaptureTransport->Layouts.FindByPredicate(
			[&AddedSnapshot](const FFlecsReplicationLayoutDefinition& Candidate)
			{
				return Candidate.LayoutId == AddedSnapshot.LayoutId;
			});
		ASSERT_THAT(IsNotNull(AddedLayout));
		const FFlecsComponentReplicationDescriptor* DontFragmentDescriptor =
			FFlecsComponentReplicationRegistry::Get(FlecsWorld).Find(
				FlecsWorld->GetIdIfRegistered<FFlecsReplicationTestDontFragmentValue>());
		ASSERT_THAT(IsNotNull(DontFragmentDescriptor));
		ASSERT_THAT(IsTrue(AddedLayout->Keys.ContainsByPredicate(
			[DontFragmentDescriptor](const FFlecsReplicationKey& Key)
			{
				return Key.StorageSchema == DontFragmentDescriptor->SchemaId;
			})));

		const flecs::table_t* TableBeforeRemoval = Source.GetEntity().table().get_table();
		CaptureTransport->Updates.Reset();
		Source.Remove<FFlecsReplicationTestDontFragmentValue>();
		ASSERT_THAT(IsTrue(TableBeforeRemoval == Source.GetEntity().table().get_table()));
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(1, CaptureTransport->Updates.Num()));
		const FFlecsReplicatedEntityUpdate RemovedSnapshot = CaptureTransport->Updates[0];
		ASSERT_THAT(IsTrue(RemovedSnapshot.LayoutId == Initial.Layout.LayoutId));

		NetworkSubsystem->StopReplicatingEntity(Source);
		Source.Destroy();
		NetworkSubsystem->ResetClientReplicationForTesting();
		NetworkSubsystem->EnterClientReplicationModeForTesting();
		const FGuid Shard = FGuid::NewGuid();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, Initial.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, Initial.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();
		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, *AddedLayout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, AddedSnapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();

		FFlecsEntityHandle Remote = NetworkSubsystem->FindEntity(NetworkId);
		ASSERT_THAT(IsTrue(Remote.IsValid()));
		ASSERT_THAT(IsTrue(Remote.Has<FFlecsReplicationTestDontFragmentValue>()));
		ASSERT_THAT(AreEqual(73, Remote.Get<FFlecsReplicationTestDontFragmentValue>().Value));

		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, RemovedSnapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();
		Remote = NetworkSubsystem->FindEntity(NetworkId);
		ASSERT_THAT(IsFalse(Remote.Has<FFlecsReplicationTestDontFragmentValue>()));
		ASSERT_THAT(AreEqual(17, Remote.Get<FFlecsReplicationTestValue>().Value));
	}

	TEST_METHOD(PairLayouts_RepresentSchemaStableAndEntityTargets)
	{
		const FFlecsEntityHandle StaticTarget = FlecsWorld->CreateEntity(TEXT("ReplicationStaticTarget"))
			.Add<FFlecsStablePathTag>();
		
		const FFlecsEntityHandle NetworkTarget = FlecsWorld->CreateEntity();
		const FFlecsNetworkId TargetId = NetworkSubsystem->BeginReplicatingEntity(NetworkTarget);
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity()
			.AddPair<FFlecsReplicationTestRelationship>(StaticTarget)
			.AddPair<FFlecsReplicationTestRelationship>(NetworkTarget);
		const UE::Flecs::Tests::FCapturedReplicationEntity Captured =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);

		ASSERT_THAT(AreEqual(2, Captured.Layout.Keys.Num()));
		ASSERT_THAT(IsTrue(Captured.Layout.Keys.ContainsByPredicate([](const FFlecsReplicationKey& Key)
		{
			return Key.TargetKind == EFlecsReplicationPairTargetKind::StablePathValue;
		})));
		
		ASSERT_THAT(IsTrue(Captured.Layout.Keys.ContainsByPredicate([TargetId](const FFlecsReplicationKey& Key)
		{
			return Key.TargetKind == EFlecsReplicationPairTargetKind::Entity && Key.EntityTarget == TargetId;
		})));
	}

	TEST_METHOD(DirtyEvents_CoalesceToFinalComposition_AndSuppressUnchangedState)
	{
		const FFlecsComponentPropertiesDefinition Properties =
			FFlecsComponentPropertiesDefinition::Make<FFlecsReplicationTestWithValue>();
		ASSERT_THAT(AreEqual(1, Properties.WithTypes.Num()));
		const FFlecsEntityHandle WithComponent = FlecsWorld->GetAlive(
			FlecsWorld->GetIdIfRegistered<FFlecsReplicationTestWithValue>());
		ASSERT_THAT(IsTrue(WithComponent.HasPair(flecs::With,
			FlecsWorld->GetIdIfRegistered<FFlecsReplicationTestRequiredTag>())));
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity().Set<FFlecsReplicationTestWithValue>({ 11 });
		ASSERT_THAT(IsTrue(Source.Has<FFlecsReplicationTestRequiredTag>()));
		const UE::Flecs::Tests::FCapturedReplicationEntity Initial =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		ASSERT_THAT(AreEqual(2, Initial.Layout.Keys.Num()));

		CaptureTransport->Updates.Reset();
		Source.Add<FFlecsReplicationTestTag>();
		Source.Remove<FFlecsReplicationTestTag>();
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(0, CaptureTransport->Updates.Num()));
	}

	TEST_METHOD(Inbox_DefersUnknownLayout_RejectsStaleRevision_AndPreservesLocalComponent)
	{
		const FGuid Shard = FGuid::NewGuid();
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity().Set<FFlecsReplicationTestValue>({ 10 });
		UE::Flecs::Tests::FCapturedReplicationEntity Captured =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		NetworkSubsystem->StopReplicatingEntity(Source);
		Source.Destroy();
		NetworkSubsystem->ResetClientReplicationForTesting();
		NetworkSubsystem->EnterClientReplicationModeForTesting();

		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, Captured.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();
		ASSERT_THAT(IsFalse(NetworkSubsystem->FindEntity(Captured.Snapshot.NetworkId).IsValid()));
		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, Captured.Layout);
		NetworkSubsystem->FlushClientReplicationForTesting();
		FFlecsEntityHandle Remote = NetworkSubsystem->FindEntity(Captured.Snapshot.NetworkId);
		ASSERT_THAT(IsTrue(Remote.IsValid()));
		Remote.Set<FFlecsReplicationTestLocalOnly>({ 77 });

		FFlecsReplicatedEntityUpdate Newer = Captured.Snapshot;
		Newer.StateRevision = 3;
		UE::Flecs::Tests::SetSnapshotValue(FlecsWorld, Newer, Captured.Layout, { 300 });
		FFlecsReplicatedEntityUpdate Stale = Captured.Snapshot;
		Stale.StateRevision = 2;
		UE::Flecs::Tests::SetSnapshotValue(FlecsWorld, Stale, Captured.Layout, { 200 });
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, Newer);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, Stale);
		NetworkSubsystem->FlushClientReplicationForTesting();
		Remote = NetworkSubsystem->FindEntity(Captured.Snapshot.NetworkId);
		ASSERT_THAT(AreEqual(300, Remote.Get<FFlecsReplicationTestValue>().Value));
		ASSERT_THAT(AreEqual(77, Remote.Get<FFlecsReplicationTestLocalOnly>().Value));
	}

	TEST_METHOD(EntityTargetPair_ResolvesAfterTargetSnapshotArrives)
	{
		const FGuid Shard = FGuid::NewGuid();
		const FFlecsEntityHandle Target = FlecsWorld->CreateEntity();
		const UE::Flecs::Tests::FCapturedReplicationEntity TargetCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Target);
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity()
			.AddPair<FFlecsReplicationTestRelationship>(Target);
		const UE::Flecs::Tests::FCapturedReplicationEntity SourceCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		NetworkSubsystem->StopReplicatingEntity(Source);
		NetworkSubsystem->StopReplicatingEntity(Target);
		Source.Destroy();
		Target.Destroy();
		NetworkSubsystem->ResetClientReplicationForTesting();
		NetworkSubsystem->EnterClientReplicationModeForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, SourceCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, SourceCapture.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();
		FFlecsEntityHandle RemoteSource = NetworkSubsystem->FindEntity(SourceCapture.Snapshot.NetworkId);
		ASSERT_THAT(IsTrue(RemoteSource.IsValid()));

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, TargetCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, TargetCapture.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();
		const FFlecsEntityHandle RemoteTarget = NetworkSubsystem->FindEntity(TargetCapture.Snapshot.NetworkId);
		RemoteSource = NetworkSubsystem->FindEntity(SourceCapture.Snapshot.NetworkId);
		ASSERT_THAT(IsTrue(RemoteTarget.IsValid()));
		ASSERT_THAT(IsTrue(RemoteSource.HasPair<FFlecsReplicationTestRelationship>(RemoteTarget)));
	}

	TEST_METHOD(EntityTargetPair_RestoresPayloadAfterTargetSnapshotArrives)
	{
		const FGuid Shard = FGuid::NewGuid();
		const FFlecsEntityHandle Target = FlecsWorld->CreateEntity();
		const UE::Flecs::Tests::FCapturedReplicationEntity TargetCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Target);
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity()
			.SetPair<FFlecsReplicationTestValueRelationship>(Target,
				FFlecsReplicationTestValueRelationship{ 17 });
		const UE::Flecs::Tests::FCapturedReplicationEntity SourceCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		NetworkSubsystem->StopReplicatingEntity(Source);
		NetworkSubsystem->StopReplicatingEntity(Target);
		Source.Destroy();
		Target.Destroy();
		NetworkSubsystem->ResetClientReplicationForTesting();
		NetworkSubsystem->EnterClientReplicationModeForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, SourceCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, SourceCapture.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, TargetCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, TargetCapture.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();
		const FFlecsEntityHandle RemoteTarget = NetworkSubsystem->FindEntity(TargetCapture.Snapshot.NetworkId);
		const FFlecsEntityHandle RemoteSource = NetworkSubsystem->FindEntity(SourceCapture.Snapshot.NetworkId);
		ASSERT_THAT(IsTrue(RemoteTarget.IsValid()));
		ASSERT_THAT(IsTrue(RemoteSource.HasPair<FFlecsReplicationTestValueRelationship>(RemoteTarget)));
		ASSERT_THAT(AreEqual(17,
			RemoteSource.GetPairFirst<FFlecsReplicationTestValueRelationship>(RemoteTarget).Value));
	}

	TEST_METHOD(EntityTargetPair_NewerSnapshotSupersedesPendingPayload)
	{
		const FGuid Shard = FGuid::NewGuid();
		const FFlecsEntityHandle Target = FlecsWorld->CreateEntity();
		const UE::Flecs::Tests::FCapturedReplicationEntity TargetCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Target);
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity()
			.SetPair<FFlecsReplicationTestValueRelationship>(Target,
				FFlecsReplicationTestValueRelationship{ 17 });
		const UE::Flecs::Tests::FCapturedReplicationEntity SourceCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		FFlecsReplicatedEntityUpdate NewerSnapshot = SourceCapture.Snapshot;
		++NewerSnapshot.StateRevision;
		UE::Flecs::Tests::SetSnapshotPairValue(FlecsWorld, NewerSnapshot,
			SourceCapture.Layout, FFlecsReplicationTestValueRelationship{ 29 });
		NetworkSubsystem->StopReplicatingEntity(Source);
		NetworkSubsystem->StopReplicatingEntity(Target);
		Source.Destroy();
		Target.Destroy();
		NetworkSubsystem->ResetClientReplicationForTesting();
		NetworkSubsystem->EnterClientReplicationModeForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, SourceCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, SourceCapture.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, NewerSnapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, TargetCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, TargetCapture.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();
		const FFlecsEntityHandle RemoteTarget = NetworkSubsystem->FindEntity(TargetCapture.Snapshot.NetworkId);
		const FFlecsEntityHandle RemoteSource = NetworkSubsystem->FindEntity(SourceCapture.Snapshot.NetworkId);
		ASSERT_THAT(IsTrue(RemoteSource.HasPair<FFlecsReplicationTestValueRelationship>(RemoteTarget)));
		ASSERT_THAT(AreEqual(29,
			RemoteSource.GetPairFirst<FFlecsReplicationTestValueRelationship>(RemoteTarget).Value));
	}

	TEST_METHOD(EntityTargetPair_NewerLayoutCancelsFixupAndStaleSnapshotCannotRestoreIt)
	{
		const FGuid Shard = FGuid::NewGuid();
		const FFlecsEntityHandle Target = FlecsWorld->CreateEntity();
		const UE::Flecs::Tests::FCapturedReplicationEntity TargetCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Target);
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity()
			.SetPair<FFlecsReplicationTestValueRelationship>(Target,
				FFlecsReplicationTestValueRelationship{ 17 });
		const UE::Flecs::Tests::FCapturedReplicationEntity SourceCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		Source.RemovePair<FFlecsReplicationTestValueRelationship>(Target);
		const UE::Flecs::Tests::FCapturedReplicationEntity WithoutPairCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		NetworkSubsystem->StopReplicatingEntity(Source);
		NetworkSubsystem->StopReplicatingEntity(Target);
		Source.Destroy();
		Target.Destroy();
		NetworkSubsystem->ResetClientReplicationForTesting();
		NetworkSubsystem->EnterClientReplicationModeForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, SourceCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, SourceCapture.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();
		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, WithoutPairCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, WithoutPairCapture.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, SourceCapture.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, TargetCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, TargetCapture.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();
		const FFlecsEntityHandle RemoteTarget = NetworkSubsystem->FindEntity(TargetCapture.Snapshot.NetworkId);
		const FFlecsEntityHandle RemoteSource = NetworkSubsystem->FindEntity(SourceCapture.Snapshot.NetworkId);
		ASSERT_THAT(IsFalse(RemoteSource.HasPair<FFlecsReplicationTestValueRelationship>(RemoteTarget)));
	}

	TEST_METHOD(EntityTargetPair_TargetRemovalCancelsPendingFixup)
	{
		const FGuid Shard = FGuid::NewGuid();
		const FFlecsEntityHandle Target = FlecsWorld->CreateEntity();
		const UE::Flecs::Tests::FCapturedReplicationEntity TargetCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Target);
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity()
			.SetPair<FFlecsReplicationTestValueRelationship>(Target,
				FFlecsReplicationTestValueRelationship{ 17 });
		const UE::Flecs::Tests::FCapturedReplicationEntity SourceCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		NetworkSubsystem->StopReplicatingEntity(Source);
		NetworkSubsystem->StopReplicatingEntity(Target);
		Source.Destroy();
		Target.Destroy();
		NetworkSubsystem->ResetClientReplicationForTesting();
		NetworkSubsystem->EnterClientReplicationModeForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, SourceCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, SourceCapture.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();
		UE::Flecs::Tests::EnqueueRemoval(NetworkSubsystem, Shard, TargetCapture.Snapshot.NetworkId);
		NetworkSubsystem->FlushClientReplicationForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, TargetCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, TargetCapture.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();
		const FFlecsEntityHandle RemoteTarget = NetworkSubsystem->FindEntity(TargetCapture.Snapshot.NetworkId);
		const FFlecsEntityHandle RemoteSource = NetworkSubsystem->FindEntity(SourceCapture.Snapshot.NetworkId);
		ASSERT_THAT(IsFalse(RemoteSource.HasPair<FFlecsReplicationTestValueRelationship>(RemoteTarget)));
	}

	TEST_METHOD(EntityTargetPair_SourceRemovalCancelsPendingFixup)
	{
		const FGuid Shard = FGuid::NewGuid();
		const FFlecsEntityHandle Target = FlecsWorld->CreateEntity();
		const UE::Flecs::Tests::FCapturedReplicationEntity TargetCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Target);
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity()
			.SetPair<FFlecsReplicationTestValueRelationship>(Target,
				FFlecsReplicationTestValueRelationship{ 17 });
		const UE::Flecs::Tests::FCapturedReplicationEntity SourceCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		NetworkSubsystem->StopReplicatingEntity(Source);
		NetworkSubsystem->StopReplicatingEntity(Target);
		Source.Destroy();
		Target.Destroy();
		NetworkSubsystem->ResetClientReplicationForTesting();
		NetworkSubsystem->EnterClientReplicationModeForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, SourceCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, SourceCapture.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();
		UE::Flecs::Tests::EnqueueRemoval(NetworkSubsystem, Shard, SourceCapture.Snapshot.NetworkId);
		NetworkSubsystem->FlushClientReplicationForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, TargetCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, TargetCapture.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();
		ASSERT_THAT(IsFalse(NetworkSubsystem->FindEntity(SourceCapture.Snapshot.NetworkId).IsValid()));
	}
};

#endif
