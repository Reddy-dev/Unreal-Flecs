// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "FlecsReplicationTestHelpers.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

FLECS_REPLICATION_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsReplicationRoutingTests,
	"UnrealFlecs.Networking.Replication.RoutingGathering",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter,
	"[Flecs][Networking][Replication]")
{
	TEST_METHOD(RouterReplacementResetAndInvalidation_PublishFullRouteBaselines)
	{
		const FFlecsEntityHandle Entity = World()->CreateEntity();
		Entity.Set<FFlecsReplicationTestValue>({ 1 });
		const UE::Flecs::Tests::FCapturedReplicationEntity Initial =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Entity);
		ASSERT_THAT(IsTrue(Initial.Snapshot.Kind == EFlecsReplicatedEntityUpdateKind::Full));

		CaptureTransport()->Snapshots.Reset();
		FName MutableRouteName(TEXT("Custom"));
		NetworkSubsystem()->SetReplicationRouter(
			MakeUnique<UE::Flecs::Tests::FTestReplicationRouter>(MutableRouteName));
		NetworkSubsystem()->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(1, CaptureTransport()->Snapshots.Num()));
		ASSERT_THAT(IsTrue(CaptureTransport()->Snapshots[0].Kind == EFlecsReplicatedEntityUpdateKind::Full));
		ASSERT_THAT(IsTrue(CaptureTransport()->Snapshots[0].Route.LogicalKey.Name == TEXT("Custom")));

		CaptureTransport()->Snapshots.Reset();
		MutableRouteName = TEXT("RoutingDirty");
		NetworkSubsystem()->MarkEntityRoutingDirty(Entity);
		NetworkSubsystem()->FlushServerReplicationForTesting();
		ASSERT_THAT(IsTrue(CaptureTransport()->Snapshots[0].Kind == EFlecsReplicatedEntityUpdateKind::Full));
		ASSERT_THAT(IsTrue(CaptureTransport()->Snapshots[0].Route.LogicalKey.Name == TEXT("RoutingDirty")));

		CaptureTransport()->Snapshots.Reset();
		NetworkSubsystem()->ResetReplicationRouter();
		NetworkSubsystem()->FlushServerReplicationForTesting();
		ASSERT_THAT(IsTrue(CaptureTransport()->Snapshots[0].Route.LogicalKey == FFlecsReplicationRouteKey::Default()));
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
		ASSERT_THAT(IsTrue(NetworkSubsystem()->IsRouteRelevant(OwnerRoute,
			FFlecsReplicationConnectionId(7), EmptyView)));
		ASSERT_THAT(IsFalse(NetworkSubsystem()->IsRouteRelevant(OwnerRoute,
			FFlecsReplicationConnectionId(8), EmptyView)));

		const FFlecsReplicationRouteDescriptor SpatialRoute = MakeFlecsSpatialCellRoute(
			FVector::ZeroVector, 100.0f, 3, 25.0f);
		FFlecsReplicationConnectionView MultiView;
		MultiView.Positions = { FVector(10000.0), FVector(110.0, 50.0, 50.0) };
		ASSERT_THAT(IsTrue(NetworkSubsystem()->IsRouteRelevant(SpatialRoute,
			FFlecsReplicationConnectionId(9), MultiView)));
		MultiView.Positions = { FVector(10000.0), FVector(126.0, 50.0, 50.0) };
		ASSERT_THAT(IsFalse(NetworkSubsystem()->IsRouteRelevant(SpatialRoute,
			FFlecsReplicationConnectionId(9), MultiView)));

		FFlecsReplicationRouteDescriptor Missing = FFlecsReplicationRouteDescriptor::Default();
		Missing.Interest.PolicyName = TEXT("Missing.Policy");
		
		TestRunner->AddExpectedError(TEXT("Rejected replication route 'Default': Interest policy 'Missing.Policy' is not registered"),
			EAutomationExpectedErrorFlags::Contains, 1, false);
		
		ASSERT_THAT(IsFalse(NetworkSubsystem()->IsRouteRelevant(Missing,
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
		NetworkSubsystem()->SetConnectionInterestFragment(Connection, Fragment);
		ASSERT_THAT(IsTrue(NetworkSubsystem()->IsRouteRelevant(Route, Connection, {})));
		ASSERT_THAT(IsTrue(NetworkSubsystem()->RemoveConnectionInterestFragment<FFlecsReplicationTestLocalOnly>(Connection)));
		ASSERT_THAT(IsFalse(NetworkSubsystem()->IsRouteRelevant(Route, Connection, {})));
		ASSERT_THAT(IsTrue(FFlecsReplicationInterestPolicyRegistry::UnregisterPolicy(TEXT("Tests.Fragment"))));
	}

	TEST_METHOD(ComponentKeyDeltas_CoalesceSuppressNoOpsAndStructuralChangesStayFull)
	{
		const FFlecsEntityHandle Entity = World()->CreateEntity();
		Entity.Set<FFlecsReplicationTestValue>({ 10 });
		UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Entity);

		CaptureTransport()->Snapshots.Reset();
		Entity.Set<FFlecsReplicationTestValue>({ 10 });
		NetworkSubsystem()->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(0, CaptureTransport()->Snapshots.Num()));

		Entity.Set<FFlecsReplicationTestValue>({ 20 });
		Entity.Set<FFlecsReplicationTestValue>({ 30 });
		NetworkSubsystem()->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(1, CaptureTransport()->Snapshots.Num()));
		ASSERT_THAT(IsTrue(CaptureTransport()->Snapshots[0].Kind == EFlecsReplicatedEntityUpdateKind::Delta));
		ASSERT_THAT(AreEqual(1, CaptureTransport()->Snapshots[0].ChangedKeys.Num()));

		CaptureTransport()->Snapshots.Reset();
		Entity.Add<FFlecsReplicationTestTag>();
		NetworkSubsystem()->MarkEntityDirty(Entity);
		NetworkSubsystem()->FlushServerReplicationForTesting();
		ASSERT_THAT(IsTrue(CaptureTransport()->Snapshots[0].Kind == EFlecsReplicatedEntityUpdateKind::Full));
	}


}; // FlecsReplicationRoutingTests

#endif
