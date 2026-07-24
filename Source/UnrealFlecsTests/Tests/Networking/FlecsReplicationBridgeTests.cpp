// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "UnrealFlecsConfigMacros.h"
#include "UnrealFlecsTests/Fixtures/FlecsReplicationFixture.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Networking/FlecsNetDirtyTag.h"
#include "Networking/FlecsReplicatedEntityComponent.h"
#include "Networking/Layout/FlecsReplicationLayoutRegistry.h"

FLECS_REPLICATION_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsReplicationBridgeTests,
	"UnrealFlecs.Networking.Replication.FakeBridge",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter,
	"[Flecs][Networking][Replication][FakeBridge]")
{
	TEST_METHOD(Fixture_InstallsAndInitializesFakeBridge)
	{
		ASSERT_THAT(IsNotNull(TestBridge()));
		ASSERT_THAT(IsTrue(TestBridge()->IsInitialized()));
		ASSERT_THAT(IsTrue(NetworkSubsystem()->GetReplicationBridge() == TestBridge()));
	}

	TEST_METHOD(DirtyObservers_ComponentAndEncodedPair_MarkReplicatedEntityDirty)
	{
		const FFlecsEntityHandle Entity = World()->CreateEntity()
			.Add<FFlecsReplicatedEntityComponent>();

		Entity.Remove<FFlecsNetDirtyTag>();
		Entity.Set<FFlecsReplicationTestValue>({ 10 });
		ASSERT_THAT(IsTrue(Entity.Has<FFlecsNetDirtyTag>()));

		Entity.Remove<FFlecsNetDirtyTag>();
		Entity.SetPair<FFlecsReplicationTestValue, FFlecsReplicationTestTag>({ 20 });
		ASSERT_THAT(IsTrue(Entity.Has<FFlecsNetDirtyTag>()));
	}

	TEST_METHOD(PublishEntityLayout_CapturesCompleteProtocolRecord)
	{
		FFlecsReplicationLayoutDefinition Layout;
		Layout.LayoutId = FFlecsReplicationLayoutRegistry::ComputeLayoutId(Layout.Keys);

		TestBridge()->PublishEntityLayout(Layout);

		ASSERT_THAT(AreEqual(1, TestBridge()->GetPublishedLayouts().Num()));
		ASSERT_THAT(IsTrue(TestBridge()->GetPublishedLayouts()[0].LayoutId == Layout.LayoutId));
		ASSERT_THAT(AreEqual(0, TestBridge()->GetPublishedLayouts()[0].Keys.Num()));
	}

	TEST_METHOD(PublishEntityLayout_WithPeer_ForwardsThroughBridgeReceivePath)
	{
		UFlecsTestReplicationBridge* Peer = NewObject<UFlecsTestReplicationBridge>(NetworkSubsystem());
		Peer->InitializeBridge();
		TestBridge()->SetPeer(Peer);

		FFlecsReplicationLayoutDefinition Layout;
		Layout.LayoutId = FFlecsReplicationLayoutRegistry::ComputeLayoutId(Layout.Keys);

		TestBridge()->PublishEntityLayout(Layout);

		ASSERT_THAT(IsNotNull(NetworkSubsystem()->GetLayoutRegistry().Find(Layout.LayoutId)));

		TestBridge()->SetPeer(nullptr);
		Peer->DeinitializeBridge();
	}

	TEST_METHOD(PublishNetEntity_CapturesIdentityLayoutValuesAndRevision)
	{
		const FFlecsNetworkId NetworkId(17, 3);
		const TArray<FFlecsReplicationKey> EmptyKeys;
		FFlecsEntityReplicationSnapshot Snapshot;
		Snapshot.LayoutId = FFlecsReplicationLayoutRegistry::ComputeLayoutId(EmptyKeys);
		Snapshot.StateRevision = 9;

		FFlecsReplicatedValue& Value = Snapshot.Values.AddDefaulted_GetRef();
		Value.KeyIndex = 4;
		Value.Bytes = { 1, 2, 3, 4 };

		TestBridge()->PublishNetEntity(NetworkId, Snapshot);

		ASSERT_THAT(AreEqual(1, TestBridge()->GetPublishedSnapshots().Num()));
		const TPair<FFlecsNetworkId, FFlecsEntityReplicationSnapshot>& Published =
			TestBridge()->GetPublishedSnapshots()[0];
		ASSERT_THAT(IsTrue(Published.Key == NetworkId));
		ASSERT_THAT(IsTrue(Published.Value.LayoutId == Snapshot.LayoutId));
		ASSERT_THAT(AreEqual(static_cast<uint32>(9), Published.Value.StateRevision));
		ASSERT_THAT(AreEqual(1, Published.Value.Values.Num()));
		ASSERT_THAT(AreEqual(static_cast<uint16>(4), Published.Value.Values[0].KeyIndex));
		ASSERT_THAT(AreEqual(4, Published.Value.Values[0].Bytes.Num()));
	}

	TEST_METHOD(PublishNetEntity_WithPeer_ForwardsIntoSubsystemReceivePath)
	{
		UFlecsTestReplicationBridge* Peer = NewObject<UFlecsTestReplicationBridge>(NetworkSubsystem());
		Peer->InitializeBridge();
		TestBridge()->SetPeer(Peer);

		FFlecsReplicationLayoutDefinition Layout;
		Layout.LayoutId = FFlecsReplicationLayoutRegistry::ComputeLayoutId(Layout.Keys);
		TestBridge()->PublishEntityLayout(Layout);

		const FFlecsNetworkId NetworkId(29, 2);
		FFlecsEntityReplicationSnapshot Baseline;
		Baseline.LayoutId = Layout.LayoutId;
		NetworkSubsystem()->GetReplicationSnapshots().Add(NetworkId, Baseline);

		FFlecsEntityReplicationSnapshot Snapshot;
		Snapshot.LayoutId = Layout.LayoutId;
		Snapshot.StateRevision = 1;
		TestBridge()->PublishNetEntity(NetworkId, Snapshot);

		const TOptional<FFlecsEntityHandle> ReceivedEntity =
			NetworkSubsystem()->GetEntityFromNetworkId(NetworkId);
		ASSERT_THAT(IsTrue(ReceivedEntity.IsSet()));
		ASSERT_THAT(IsTrue(ReceivedEntity.GetValue().IsValid()));

		TestBridge()->SetPeer(nullptr);
		Peer->DeinitializeBridge();
	}

	TEST_METHOD(ResetCapturedRecords_ClearsLayoutsAndSnapshotsWithoutDeinitializing)
	{
		FFlecsReplicationLayoutDefinition Layout;
		Layout.LayoutId = FFlecsReplicationLayoutRegistry::ComputeLayoutId(Layout.Keys);
		TestBridge()->PublishEntityLayout(Layout);
		TestBridge()->PublishNetEntity(FFlecsNetworkId(2, 1), FFlecsEntityReplicationSnapshot());

		TestBridge()->ResetCapturedRecords();

		ASSERT_THAT(AreEqual(0, TestBridge()->GetPublishedLayouts().Num()));
		ASSERT_THAT(AreEqual(0, TestBridge()->GetPublishedSnapshots().Num()));
		ASSERT_THAT(IsTrue(TestBridge()->IsInitialized()));
	}

	TEST_METHOD(SetReplicationBridgeForTesting_TransitionsBridgeLifecycle)
	{
		UFlecsTestReplicationBridge* Original = TestBridge();
		UFlecsTestReplicationBridge* Replacement =
			NewObject<UFlecsTestReplicationBridge>(NetworkSubsystem());

		NetworkSubsystem()->SetReplicationBridgeForTesting(Replacement);

		ASSERT_THAT(IsFalse(Original->IsInitialized()));
		ASSERT_THAT(IsTrue(Replacement->IsInitialized()));
		ASSERT_THAT(IsTrue(NetworkSubsystem()->GetReplicationBridge() == Replacement));

		NetworkSubsystem()->SetReplicationBridgeForTesting(Original);

		ASSERT_THAT(IsFalse(Replacement->IsInitialized()));
		ASSERT_THAT(IsTrue(Original->IsInitialized()));
	}
}; // FlecsReplicationBridgeTests

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS
