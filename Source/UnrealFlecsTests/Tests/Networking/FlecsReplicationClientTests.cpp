// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "FlecsReplicationTestHelpers.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

FLECS_REPLICATION_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsReplicationClientTests,
	"UnrealFlecs.Networking.Replication.ClientApplication.Inbox",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter,
	"[Flecs][Networking][Replication]")
{
	TEST_METHOD(Inbox_DefersUnknownLayout_RejectsStaleRevision_AndPreservesLocalComponent)
	{
		const FGuid Shard = FGuid::NewGuid();
		const FFlecsEntityHandle Source = World()->CreateEntity().Set<FFlecsReplicationTestValue>({ 10 });
		UE::Flecs::Tests::FCapturedReplicationEntity Captured =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Source);
		NetworkSubsystem()->StopReplicatingEntity(Source);
		Source.Destroy();
		NetworkSubsystem()->ResetClientReplicationForTesting();
		NetworkSubsystem()->EnterClientReplicationModeForTesting();

		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, Captured.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();
		ASSERT_THAT(IsFalse(NetworkSubsystem()->FindEntity(Captured.Snapshot.NetworkId).IsValid()));
		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), Shard, Captured.Layout);
		NetworkSubsystem()->FlushClientReplicationForTesting();
		FFlecsEntityHandle Remote = NetworkSubsystem()->FindEntity(Captured.Snapshot.NetworkId);
		ASSERT_THAT(IsTrue(Remote.IsValid()));
		Remote.Set<FFlecsReplicationTestLocalOnly>({ 77 });

		FFlecsReplicatedEntityUpdate Newer = Captured.Snapshot;
		Newer.StateRevision = 3;
		UE::Flecs::Tests::SetSnapshotValue(World(), Newer, Captured.Layout, { 300 });
		FFlecsReplicatedEntityUpdate Stale = Captured.Snapshot;
		Stale.StateRevision = 2;
		UE::Flecs::Tests::SetSnapshotValue(World(), Stale, Captured.Layout, { 200 });
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, Newer);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, Stale);
		NetworkSubsystem()->FlushClientReplicationForTesting();
		Remote = NetworkSubsystem()->FindEntity(Captured.Snapshot.NetworkId);
		ASSERT_THAT(AreEqual(300, Remote.Get<FFlecsReplicationTestValue>().Value));
		ASSERT_THAT(AreEqual(77, Remote.Get<FFlecsReplicationTestLocalOnly>().Value));
	}

	TEST_METHOD(MigrationSourceOwnership_RejectsLateOldPageRemoval)
	{
		const FGuid OldSource = FGuid::NewGuid();
		const FGuid NewSource = FGuid::NewGuid();
		const FFlecsEntityHandle Source = World()->CreateEntity().Set<FFlecsReplicationTestValue>({ 10 });
		const UE::Flecs::Tests::FCapturedReplicationEntity Captured =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Source);
		NetworkSubsystem()->StopReplicatingEntity(Source);
		Source.Destroy();
		NetworkSubsystem()->ResetClientReplicationForTesting();
		NetworkSubsystem()->EnterClientReplicationModeForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), OldSource, Captured.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), OldSource, Captured.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();

		FFlecsReplicatedEntityUpdate Migrated = Captured.Snapshot;
		Migrated.StateRevision++;
		UE::Flecs::Tests::SetSnapshotValue(World(), Migrated, Captured.Layout, { 99 });
		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), NewSource, Captured.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), NewSource, Migrated);
		UE::Flecs::Tests::EnqueueRemoval(NetworkSubsystem(), OldSource, Migrated.NetworkId);
		NetworkSubsystem()->FlushClientReplicationForTesting();

		const FFlecsEntityHandle Remote = NetworkSubsystem()->FindEntity(Migrated.NetworkId);
		ASSERT_THAT(IsTrue(Remote.IsValid()));
		ASSERT_THAT(AreEqual(99, Remote.Get<FFlecsReplicationTestValue>().Value));
	}

	TEST_METHOD(DeltaWithoutBaseline_DefersThenAppliesAndRejectsStaleDelta)
	{
		const FGuid SourceShard = FGuid::NewGuid();
		const FFlecsEntityHandle Source = World()->CreateEntity().Set<FFlecsReplicationTestValue>({ 10 });
		const UE::Flecs::Tests::FCapturedReplicationEntity Captured =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Source);
		NetworkSubsystem()->StopReplicatingEntity(Source);
		Source.Destroy();
		NetworkSubsystem()->ResetClientReplicationForTesting();
		NetworkSubsystem()->EnterClientReplicationModeForTesting();

		FFlecsReplicatedEntityUpdate Delta = Captured.Snapshot;
		Delta.Kind = EFlecsReplicatedEntityUpdateKind::Delta;
		Delta.StateRevision = 3;
		Delta.ChangedKeys = { Delta.Values[0].KeyIndex };
		UE::Flecs::Tests::SetSnapshotValue(World(), Delta, Captured.Layout, { 300 });
		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), SourceShard, Captured.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), SourceShard, Delta);
		NetworkSubsystem()->FlushClientReplicationForTesting();
		ASSERT_THAT(IsFalse(NetworkSubsystem()->FindEntity(Delta.NetworkId).IsValid()));

		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), SourceShard, Captured.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();
		FFlecsEntityHandle Remote = NetworkSubsystem()->FindEntity(Delta.NetworkId);
		ASSERT_THAT(IsTrue(Remote.IsValid()));
		ASSERT_THAT(AreEqual(300, Remote.Get<FFlecsReplicationTestValue>().Value));

		FFlecsReplicatedEntityUpdate StaleDelta = Delta;
		StaleDelta.StateRevision = 2;
		UE::Flecs::Tests::SetSnapshotValue(World(), StaleDelta, Captured.Layout, { 200 });
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), SourceShard, StaleDelta);
		NetworkSubsystem()->FlushClientReplicationForTesting();
		Remote = NetworkSubsystem()->FindEntity(Delta.NetworkId);
		ASSERT_THAT(AreEqual(300, Remote.Get<FFlecsReplicationTestValue>().Value));
	}

}; // FlecsReplicationClientTests

#endif
