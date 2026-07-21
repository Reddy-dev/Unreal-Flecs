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
#include "Networking/FlecsReplicatedTrait.h"
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

	class FTestReplicationRouter final : public IFlecsReplicationRouter
	{
	public:
		explicit FTestReplicationRouter(const FName& InRouteName)
			: RouteName(&InRouteName)
		{
		}

		virtual FFlecsReplicationRouteDescriptor Route(const FFlecsEntityHandle&) const override
		{
			FFlecsReplicationRouteDescriptor Result = FFlecsReplicationRouteDescriptor::Default();
			Result.LogicalKey = FFlecsReplicationRouteKey(*RouteName);
			return Result;
		}

	private:
		const FName* RouteName;
	}; // class FTestReplicationRouter

	class FTestFragmentInterestPolicy final
		: public TFlecsReplicationInterestPolicy<FFlecsReplicationEveryoneInterestDescriptor>
	{
	public:
		FTestFragmentInterestPolicy()
			: TFlecsReplicationInterestPolicy(TEXT("Tests.Fragment"))
		{
		}

	protected:
		virtual bool IsInterested(const FFlecsReplicationEveryoneInterestDescriptor&,
			const FFlecsReplicationInterestEvaluationQuery& Query) const override
		{
			const FFlecsReplicationTestLocalOnly* Fragment =
				Query.Context.Find<FFlecsReplicationTestLocalOnly>();
			return Fragment && Fragment->Value == 42;
		}
	}; // class FTestFragmentInterestPolicy

	class FTestInvalidInterestPolicy final
		: public TFlecsReplicationInterestPolicy<FFlecsReplicationTestInvalidInterestDescriptor>
	{
	public:
		FTestInvalidInterestPolicy()
			: TFlecsReplicationInterestPolicy(TEXT("Tests.InvalidReference"))
		{
		}

	protected:
		virtual bool IsInterested(const FFlecsReplicationTestInvalidInterestDescriptor&,
			const FFlecsReplicationInterestEvaluationQuery&) const override
		{
			return true;
		}
	}; // class FTestInvalidInterestPolicy

	template<typename T>
	FFlecsComponentHandle RegisterTestComponent(UFlecsWorld* World)
	{
		const FFlecsComponentHandle Component = World->RegisterComponentType<T>();
		const FFlecsComponentPropertiesDefinition Properties = FFlecsComponentPropertiesDefinition::Make<T>();
		Properties.PropertiesFunction(World, Component, Properties);
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
		Transport->Snapshots.Reset();
		const FFlecsNetworkId NetworkId = Subsystem->BeginReplicatingEntity(Entity);
		Subsystem->FlushServerReplicationForTesting();
		const FFlecsReplicatedEntityUpdate* FoundSnapshot = Transport->Snapshots.FindByPredicate(
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
				&& Layout.Keys[Serialized.KeyIndex].TryGetStorageDescriptor(World) == Descriptor)
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
				&& Layout.Keys[Serialized.KeyIndex].TryGetStorageDescriptor(World) == Descriptor)
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

	TEST_METHOD(RouterReplacementResetAndInvalidation_PublishFullRouteBaselines)
	{
		const FFlecsEntityHandle Entity = FlecsWorld->CreateEntity();
		Entity.Set<FFlecsReplicationTestValue>({ 1 });
		const UE::Flecs::Tests::FCapturedReplicationEntity Initial =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Entity);
		ASSERT_THAT(IsTrue(Initial.Snapshot.Kind == EFlecsReplicatedEntityUpdateKind::Full));

		CaptureTransport->Snapshots.Reset();
		FName MutableRouteName(TEXT("Custom"));
		NetworkSubsystem->SetReplicationRouter(
			MakeUnique<UE::Flecs::Tests::FTestReplicationRouter>(MutableRouteName));
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(1, CaptureTransport->Snapshots.Num()));
		ASSERT_THAT(IsTrue(CaptureTransport->Snapshots[0].Kind == EFlecsReplicatedEntityUpdateKind::Full));
		ASSERT_THAT(IsTrue(CaptureTransport->Snapshots[0].Route.LogicalKey.Name == TEXT("Custom")));

		CaptureTransport->Snapshots.Reset();
		MutableRouteName = TEXT("RoutingDirty");
		NetworkSubsystem->MarkEntityRoutingDirty(Entity);
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(IsTrue(CaptureTransport->Snapshots[0].Kind == EFlecsReplicatedEntityUpdateKind::Full));
		ASSERT_THAT(IsTrue(CaptureTransport->Snapshots[0].Route.LogicalKey.Name == TEXT("RoutingDirty")));

		CaptureTransport->Snapshots.Reset();
		NetworkSubsystem->ResetReplicationRouter();
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(IsTrue(CaptureTransport->Snapshots[0].Route.LogicalKey == FFlecsReplicationRouteKey::Default()));
	}

	TEST_METHOD(SpatialCellsOwnerAndMultiViewPolicies_AreDeterministicAndFailClosed)
	{
		ASSERT_THAT(IsTrue(FlecsReplicationSpatialCell(FVector(-0.1, -1000.0, 999.9), 1000.0f)
			== FIntVector(-1, -1, 0)));

		FFlecsReplicationRouteDescriptor OwnerRoute = FFlecsReplicationRouteDescriptor::Default();
		FFlecsReplicationOwnerInterestDescriptor OwnerDescriptor;
		OwnerDescriptor.OwnerConnection = FFlecsReplicationConnectionId(7);
		OwnerRoute.Interest = FFlecsReplicationInterestBinding::Make(
			FFlecsReplicationInterestPolicyNames::Owner, OwnerDescriptor);
		const FFlecsReplicationConnectionView EmptyView;
		ASSERT_THAT(IsTrue(NetworkSubsystem->IsRouteRelevant(OwnerRoute,
			FFlecsReplicationConnectionId(7), EmptyView)));
		ASSERT_THAT(IsFalse(NetworkSubsystem->IsRouteRelevant(OwnerRoute,
			FFlecsReplicationConnectionId(8), EmptyView)));

		const FFlecsReplicationRouteDescriptor SpatialRoute = MakeFlecsSpatialCellRoute(
			FVector::ZeroVector, 100.0f, 3, 25.0f);
		FFlecsReplicationConnectionView MultiView;
		MultiView.Positions = { FVector(10000.0), FVector(110.0, 50.0, 50.0) };
		ASSERT_THAT(IsTrue(NetworkSubsystem->IsRouteRelevant(SpatialRoute,
			FFlecsReplicationConnectionId(9), MultiView)));
		MultiView.Positions = { FVector(10000.0), FVector(126.0, 50.0, 50.0) };
		ASSERT_THAT(IsFalse(NetworkSubsystem->IsRouteRelevant(SpatialRoute,
			FFlecsReplicationConnectionId(9), MultiView)));

		FFlecsReplicationRouteDescriptor Missing = FFlecsReplicationRouteDescriptor::Default();
		Missing.Interest.PolicyName = TEXT("Missing.Policy");
		
		TestRunner->AddExpectedError(TEXT("Rejected replication route 'Default': Interest policy 'Missing.Policy' is not registered"),
			EAutomationExpectedErrorFlags::Contains, 1, false);
		
		ASSERT_THAT(IsFalse(NetworkSubsystem->IsRouteRelevant(Missing,
			FFlecsReplicationConnectionId(9), EmptyView)));
	}

	TEST_METHOD(CustomPolicyRegistrationAndTypedConnectionFragments_AreModular)
	{
		FString Error;
		const FFlecsReplicationInterestBinding MissingBinding = FFlecsReplicationInterestBinding::Make(
			TEXT("Tests.Fragment"), FFlecsReplicationEveryoneInterestDescriptor{});
		ASSERT_THAT(IsFalse(FFlecsReplicationInterestPolicyRegistry::ValidateBinding(MissingBinding, Error)));
		ASSERT_THAT(IsTrue(FFlecsReplicationInterestPolicyRegistry::RegisterPolicy(
			MakeUnique<UE::Flecs::Tests::FTestFragmentInterestPolicy>())));
		ASSERT_THAT(IsFalse(FFlecsReplicationInterestPolicyRegistry::RegisterPolicy(
			MakeUnique<UE::Flecs::Tests::FTestFragmentInterestPolicy>())));
		ASSERT_THAT(IsTrue(FFlecsReplicationInterestPolicyRegistry::ValidateBinding(MissingBinding, Error)));
		const FFlecsReplicationInterestBinding WrongType = FFlecsReplicationInterestBinding::Make(
			TEXT("Tests.Fragment"), FFlecsReplicationSpatialCellInterestDescriptor{});
		ASSERT_THAT(IsFalse(FFlecsReplicationInterestPolicyRegistry::ValidateBinding(WrongType, Error)));

		ASSERT_THAT(IsTrue(FFlecsReplicationInterestPolicyRegistry::RegisterPolicy(
			MakeUnique<UE::Flecs::Tests::FTestInvalidInterestPolicy>())));
		const FFlecsReplicationInterestBinding InvalidReference = FFlecsReplicationInterestBinding::Make(
			TEXT("Tests.InvalidReference"), FFlecsReplicationTestInvalidInterestDescriptor{});
		ASSERT_THAT(IsFalse(FFlecsReplicationInterestPolicyRegistry::ValidateBinding(InvalidReference, Error)));
		ASSERT_THAT(IsTrue(FFlecsReplicationInterestPolicyRegistry::UnregisterPolicy(
			TEXT("Tests.InvalidReference"))));

		FFlecsReplicationRouteDescriptor Route = FFlecsReplicationRouteDescriptor::Default();
		Route.Interest = FFlecsReplicationInterestBinding::Make(TEXT("Tests.Fragment"),
			FFlecsReplicationEveryoneInterestDescriptor{});
		const FFlecsReplicationConnectionId Connection(44);
		FFlecsReplicationTestLocalOnly Fragment;
		Fragment.Value = 42;
		NetworkSubsystem->SetConnectionInterestFragment(Connection, Fragment);
		ASSERT_THAT(IsTrue(NetworkSubsystem->IsRouteRelevant(Route, Connection, {})));
		ASSERT_THAT(IsTrue(NetworkSubsystem->RemoveConnectionInterestFragment<FFlecsReplicationTestLocalOnly>(Connection)));
		ASSERT_THAT(IsFalse(NetworkSubsystem->IsRouteRelevant(Route, Connection, {})));
		ASSERT_THAT(IsTrue(FFlecsReplicationInterestPolicyRegistry::UnregisterPolicy(TEXT("Tests.Fragment"))));
	}

	TEST_METHOD(ComponentKeyDeltas_CoalesceSuppressNoOpsAndStructuralChangesStayFull)
	{
		const FFlecsEntityHandle Entity = FlecsWorld->CreateEntity();
		Entity.Set<FFlecsReplicationTestValue>({ 10 });
		UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Entity);

		CaptureTransport->Snapshots.Reset();
		Entity.Set<FFlecsReplicationTestValue>({ 10 });
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(0, CaptureTransport->Snapshots.Num()));

		Entity.Set<FFlecsReplicationTestValue>({ 20 });
		Entity.Set<FFlecsReplicationTestValue>({ 30 });
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(1, CaptureTransport->Snapshots.Num()));
		ASSERT_THAT(IsTrue(CaptureTransport->Snapshots[0].Kind == EFlecsReplicatedEntityUpdateKind::Delta));
		ASSERT_THAT(AreEqual(1, CaptureTransport->Snapshots[0].ChangedKeys.Num()));

		CaptureTransport->Snapshots.Reset();
		Entity.Add<FFlecsReplicationTestTag>();
		NetworkSubsystem->MarkEntityDirty(Entity);
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(IsTrue(CaptureTransport->Snapshots[0].Kind == EFlecsReplicatedEntityUpdateKind::Full));
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

	/*TEST_METHOD(DontFragmentComponent_ReplicatesRuntimeAdditionValueAndRemoval)
	{
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity()
			.Set<FFlecsReplicationTestValue>({ 17 });
		const UE::Flecs::Tests::FCapturedReplicationEntity Initial =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		const FFlecsNetworkId NetworkId = Initial.Snapshot.NetworkId;

		const flecs::table_t* TableBeforeAddition = Source.GetEntity().table().get_table();
		CaptureTransport->Snapshots.Reset();
		Source.Set<FFlecsReplicationTestDontFragmentValue>({ 73 });
		ASSERT_THAT(IsTrue(TableBeforeAddition == Source.GetEntity().table().get_table()));
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(1, CaptureTransport->Snapshots.Num()));
		const FFlecsReplicatedEntityUpdate AddedSnapshot = CaptureTransport->Snapshots[0];
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
			[FlecsWorld = FlecsWorld, DontFragmentDescriptor](const FFlecsReplicationKey& Key)
			{
				return Key.TryGetStorageDescriptor(FlecsWorld) == DontFragmentDescriptor;
			})));

		const flecs::table_t* TableBeforeRemoval = Source.GetEntity().table().get_table();
		CaptureTransport->Snapshots.Reset();
		Source.Remove<FFlecsReplicationTestDontFragmentValue>();
		ASSERT_THAT(IsTrue(TableBeforeRemoval == Source.GetEntity().table().get_table()));
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(1, CaptureTransport->Snapshots.Num()));
		const FFlecsReplicatedEntityUpdate RemovedSnapshot = CaptureTransport->Snapshots[0];
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
	*/

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
			return Key.Secondary.Kind == EFlecsReplicationPairTargetKind::StablePathValue;
		})));
		
		ASSERT_THAT(IsTrue(Captured.Layout.Keys.ContainsByPredicate([TargetId](const FFlecsReplicationKey& Key)
		{
			return Key.Secondary.Kind == EFlecsReplicationPairTargetKind::Entity
				&& Key.Secondary.Entity == TargetId;
		})));
	}

	TEST_METHOD(DescriptorFreeIndividuals_ResolveStandaloneAndPairStructure)
	{
		const FGuid Shard = FGuid::NewGuid();
		const FFlecsEntityHandle SymbolPrimary = FlecsWorld->CreateEntity()
			.Add<FFlecsReplicatedTrait>();
		SymbolPrimary.GetEntity().set_symbol("ReplicationSymbolPrimary");
		
		const FFlecsEntityHandle SymbolSecondary = FlecsWorld->CreateEntity();
		SymbolSecondary.GetEntity().set_symbol("ReplicationSymbolSecondary");
		
		const FFlecsEntityHandle PathPrimary = FlecsWorld->CreateEntity(TEXT("ReplicationPathPrimary"))
			.Add<FFlecsStablePathTag>()
			.Add<FFlecsReplicatedTrait>();
		const FFlecsEntityHandle PathSecondary = FlecsWorld->CreateEntity(TEXT("ReplicationPathSecondary"))
			.Add<FFlecsStablePathTag>();

		const FFlecsEntityHandle NetworkPrimary = FlecsWorld->CreateEntity();
		const UE::Flecs::Tests::FCapturedReplicationEntity NetworkPrimaryCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, NetworkPrimary);
		const FFlecsEntityHandle NetworkSecondary = FlecsWorld->CreateEntity();
		const UE::Flecs::Tests::FCapturedReplicationEntity NetworkSecondaryCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, NetworkSecondary);

		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity()
			.Add(NetworkPrimary)
			.Add(SymbolPrimary)
			.Add(PathPrimary)
			.AddPair(NetworkPrimary, NetworkSecondary)
			.AddPair(SymbolPrimary, SymbolSecondary)
			.AddPair(PathPrimary, PathSecondary);
		
		const UE::Flecs::Tests::FCapturedReplicationEntity SourceCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);

		ASSERT_THAT(AreEqual(6, SourceCapture.Layout.Keys.Num()));
		for (const FFlecsReplicationKey& Key : SourceCapture.Layout.Keys)
		{
			ASSERT_THAT(IsTrue(Key.StorageKind == EFlecsReplicationKeyStorageKind::None));
			ASSERT_THAT(IsNull(Key.TryGetStorageDescriptor(FlecsWorld)));
		}

		NetworkSubsystem->StopReplicatingEntity(Source);
		NetworkSubsystem->StopReplicatingEntity(NetworkPrimary);
		NetworkSubsystem->StopReplicatingEntity(NetworkSecondary);
		Source.Destroy();
		NetworkPrimary.Destroy();
		NetworkSecondary.Destroy();
		NetworkSubsystem->ResetClientReplicationForTesting();
		NetworkSubsystem->EnterClientReplicationModeForTesting();
		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, NetworkPrimaryCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, NetworkPrimaryCapture.Snapshot);
		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, NetworkSecondaryCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, NetworkSecondaryCapture.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();

		const FFlecsEntityHandle RemoteNetworkPrimary = NetworkSubsystem->FindEntity(
			NetworkPrimaryCapture.Snapshot.NetworkId);
		const FFlecsEntityHandle RemoteNetworkSecondary = NetworkSubsystem->FindEntity(
			NetworkSecondaryCapture.Snapshot.NetworkId);
		ASSERT_THAT(IsTrue(RemoteNetworkPrimary.IsValid()));
		ASSERT_THAT(IsTrue(RemoteNetworkSecondary.IsValid()));

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, Shard, SourceCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, Shard, SourceCapture.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();

		const FFlecsEntityHandle RemoteSource = NetworkSubsystem->FindEntity(SourceCapture.Snapshot.NetworkId);
		ASSERT_THAT(IsTrue(RemoteSource.IsValid()));
		ASSERT_THAT(IsTrue(RemoteSource.Has(RemoteNetworkPrimary)));
		ASSERT_THAT(IsTrue(RemoteSource.Has(SymbolPrimary)));
		ASSERT_THAT(IsTrue(RemoteSource.Has(PathPrimary)));
		ASSERT_THAT(IsTrue(RemoteSource.HasPair(RemoteNetworkPrimary, RemoteNetworkSecondary)));
		ASSERT_THAT(IsTrue(RemoteSource.HasPair(SymbolPrimary, SymbolSecondary)));
		ASSERT_THAT(IsTrue(RemoteSource.HasPair(PathPrimary, PathSecondary)));
	}

	TEST_METHOD(DirtyEvents_CoalesceToFinalComposition_AndIncludeWithTypes)
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

		CaptureTransport->Snapshots.Reset();
		Source.Add<FFlecsReplicationTestTag>();
		Source.Remove<FFlecsReplicationTestTag>();
		NetworkSubsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(0, CaptureTransport->Snapshots.Num()));
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

	TEST_METHOD(MigrationSourceOwnership_RejectsLateOldPageRemoval)
	{
		const FGuid OldSource = FGuid::NewGuid();
		const FGuid NewSource = FGuid::NewGuid();
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity().Set<FFlecsReplicationTestValue>({ 10 });
		const UE::Flecs::Tests::FCapturedReplicationEntity Captured =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		NetworkSubsystem->StopReplicatingEntity(Source);
		Source.Destroy();
		NetworkSubsystem->ResetClientReplicationForTesting();
		NetworkSubsystem->EnterClientReplicationModeForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, OldSource, Captured.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, OldSource, Captured.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();

		FFlecsReplicatedEntityUpdate Migrated = Captured.Snapshot;
		Migrated.StateRevision++;
		UE::Flecs::Tests::SetSnapshotValue(FlecsWorld, Migrated, Captured.Layout, { 99 });
		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, NewSource, Captured.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, NewSource, Migrated);
		UE::Flecs::Tests::EnqueueRemoval(NetworkSubsystem, OldSource, Migrated.NetworkId);
		NetworkSubsystem->FlushClientReplicationForTesting();

		const FFlecsEntityHandle Remote = NetworkSubsystem->FindEntity(Migrated.NetworkId);
		ASSERT_THAT(IsTrue(Remote.IsValid()));
		ASSERT_THAT(AreEqual(99, Remote.Get<FFlecsReplicationTestValue>().Value));
	}

	TEST_METHOD(DeltaWithoutBaseline_DefersThenAppliesAndRejectsStaleDelta)
	{
		const FGuid SourceShard = FGuid::NewGuid();
		const FFlecsEntityHandle Source = FlecsWorld->CreateEntity().Set<FFlecsReplicationTestValue>({ 10 });
		const UE::Flecs::Tests::FCapturedReplicationEntity Captured =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem, CaptureTransport, Source);
		NetworkSubsystem->StopReplicatingEntity(Source);
		Source.Destroy();
		NetworkSubsystem->ResetClientReplicationForTesting();
		NetworkSubsystem->EnterClientReplicationModeForTesting();

		FFlecsReplicatedEntityUpdate Delta = Captured.Snapshot;
		Delta.Kind = EFlecsReplicatedEntityUpdateKind::Delta;
		Delta.StateRevision = 3;
		Delta.ChangedKeys = { Delta.Values[0].KeyIndex };
		UE::Flecs::Tests::SetSnapshotValue(FlecsWorld, Delta, Captured.Layout, { 300 });
		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem, SourceShard, Captured.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, SourceShard, Delta);
		NetworkSubsystem->FlushClientReplicationForTesting();
		ASSERT_THAT(IsFalse(NetworkSubsystem->FindEntity(Delta.NetworkId).IsValid()));

		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, SourceShard, Captured.Snapshot);
		NetworkSubsystem->FlushClientReplicationForTesting();
		FFlecsEntityHandle Remote = NetworkSubsystem->FindEntity(Delta.NetworkId);
		ASSERT_THAT(IsTrue(Remote.IsValid()));
		ASSERT_THAT(AreEqual(300, Remote.Get<FFlecsReplicationTestValue>().Value));

		FFlecsReplicatedEntityUpdate StaleDelta = Delta;
		StaleDelta.StateRevision = 2;
		UE::Flecs::Tests::SetSnapshotValue(FlecsWorld, StaleDelta, Captured.Layout, { 200 });
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem, SourceShard, StaleDelta);
		NetworkSubsystem->FlushClientReplicationForTesting();
		Remote = NetworkSubsystem->FindEntity(Delta.NetworkId);
		ASSERT_THAT(AreEqual(300, Remote.Get<FFlecsReplicationTestValue>().Value));
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
