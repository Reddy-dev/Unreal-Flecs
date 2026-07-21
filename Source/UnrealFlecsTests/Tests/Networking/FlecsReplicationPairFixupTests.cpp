// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "FlecsReplicationTestHelpers.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

FLECS_REPLICATION_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsReplicationPairFixupTests,
	"UnrealFlecs.Networking.Replication.ClientApplication.PairFixups",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter,
	"[Flecs][Networking][Replication]")
{
	TEST_METHOD(EntityTargetPair_ResolvesAfterTargetSnapshotArrives)
	{
		const FGuid Shard = FGuid::NewGuid();
		const FFlecsEntityHandle Target = World()->CreateEntity();
		const UE::Flecs::Tests::FCapturedReplicationEntity TargetCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Target);
		const FFlecsEntityHandle Source = World()->CreateEntity()
			.AddPair<FFlecsReplicationTestRelationship>(Target);
		const UE::Flecs::Tests::FCapturedReplicationEntity SourceCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Source);
		NetworkSubsystem()->StopReplicatingEntity(Source);
		NetworkSubsystem()->StopReplicatingEntity(Target);
		Source.Destroy();
		Target.Destroy();
		NetworkSubsystem()->ResetClientReplicationForTesting();
		NetworkSubsystem()->EnterClientReplicationModeForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), Shard, SourceCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, SourceCapture.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();
		FFlecsEntityHandle RemoteSource = NetworkSubsystem()->FindEntity(SourceCapture.Snapshot.NetworkId);
		ASSERT_THAT(IsTrue(RemoteSource.IsValid()));

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), Shard, TargetCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, TargetCapture.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();
		const FFlecsEntityHandle RemoteTarget = NetworkSubsystem()->FindEntity(TargetCapture.Snapshot.NetworkId);
		RemoteSource = NetworkSubsystem()->FindEntity(SourceCapture.Snapshot.NetworkId);
		ASSERT_THAT(IsTrue(RemoteTarget.IsValid()));
		ASSERT_THAT(IsTrue(RemoteSource.HasPair<FFlecsReplicationTestRelationship>(RemoteTarget)));
	}

	TEST_METHOD(EntityTargetPair_RestoresPayloadAfterTargetSnapshotArrives)
	{
		const FGuid Shard = FGuid::NewGuid();
		const FFlecsEntityHandle Target = World()->CreateEntity();
		const UE::Flecs::Tests::FCapturedReplicationEntity TargetCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Target);
		const FFlecsEntityHandle Source = World()->CreateEntity()
			.SetPair<FFlecsReplicationTestValueRelationship>(Target,
				FFlecsReplicationTestValueRelationship{ 17 });
		const UE::Flecs::Tests::FCapturedReplicationEntity SourceCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Source);
		NetworkSubsystem()->StopReplicatingEntity(Source);
		NetworkSubsystem()->StopReplicatingEntity(Target);
		Source.Destroy();
		Target.Destroy();
		NetworkSubsystem()->ResetClientReplicationForTesting();
		NetworkSubsystem()->EnterClientReplicationModeForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), Shard, SourceCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, SourceCapture.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), Shard, TargetCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, TargetCapture.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();
		const FFlecsEntityHandle RemoteTarget = NetworkSubsystem()->FindEntity(TargetCapture.Snapshot.NetworkId);
		const FFlecsEntityHandle RemoteSource = NetworkSubsystem()->FindEntity(SourceCapture.Snapshot.NetworkId);
		ASSERT_THAT(IsTrue(RemoteTarget.IsValid()));
		ASSERT_THAT(IsTrue(RemoteSource.HasPair<FFlecsReplicationTestValueRelationship>(RemoteTarget)));
		ASSERT_THAT(AreEqual(17,
			RemoteSource.GetPairFirst<FFlecsReplicationTestValueRelationship>(RemoteTarget).Value));
	}

	TEST_METHOD(EntityTargetPair_NewerSnapshotSupersedesPendingPayload)
	{
		const FGuid Shard = FGuid::NewGuid();
		const FFlecsEntityHandle Target = World()->CreateEntity();
		const UE::Flecs::Tests::FCapturedReplicationEntity TargetCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Target);
		const FFlecsEntityHandle Source = World()->CreateEntity()
			.SetPair<FFlecsReplicationTestValueRelationship>(Target,
				FFlecsReplicationTestValueRelationship{ 17 });
		const UE::Flecs::Tests::FCapturedReplicationEntity SourceCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Source);
		FFlecsReplicatedEntityUpdate NewerSnapshot = SourceCapture.Snapshot;
		++NewerSnapshot.StateRevision;
		UE::Flecs::Tests::SetSnapshotPairValue(World(), NewerSnapshot,
			SourceCapture.Layout, FFlecsReplicationTestValueRelationship{ 29 });
		NetworkSubsystem()->StopReplicatingEntity(Source);
		NetworkSubsystem()->StopReplicatingEntity(Target);
		Source.Destroy();
		Target.Destroy();
		NetworkSubsystem()->ResetClientReplicationForTesting();
		NetworkSubsystem()->EnterClientReplicationModeForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), Shard, SourceCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, SourceCapture.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, NewerSnapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), Shard, TargetCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, TargetCapture.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();
		const FFlecsEntityHandle RemoteTarget = NetworkSubsystem()->FindEntity(TargetCapture.Snapshot.NetworkId);
		const FFlecsEntityHandle RemoteSource = NetworkSubsystem()->FindEntity(SourceCapture.Snapshot.NetworkId);
		ASSERT_THAT(IsTrue(RemoteSource.HasPair<FFlecsReplicationTestValueRelationship>(RemoteTarget)));
		ASSERT_THAT(AreEqual(29,
			RemoteSource.GetPairFirst<FFlecsReplicationTestValueRelationship>(RemoteTarget).Value));
	}

	TEST_METHOD(EntityTargetPair_NewerLayoutCancelsFixupAndStaleSnapshotCannotRestoreIt)
	{
		const FGuid Shard = FGuid::NewGuid();
		const FFlecsEntityHandle Target = World()->CreateEntity();
		const UE::Flecs::Tests::FCapturedReplicationEntity TargetCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Target);
		const FFlecsEntityHandle Source = World()->CreateEntity()
			.SetPair<FFlecsReplicationTestValueRelationship>(Target,
				FFlecsReplicationTestValueRelationship{ 17 });
		const UE::Flecs::Tests::FCapturedReplicationEntity SourceCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Source);
		Source.RemovePair<FFlecsReplicationTestValueRelationship>(Target);
		const UE::Flecs::Tests::FCapturedReplicationEntity WithoutPairCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Source);
		NetworkSubsystem()->StopReplicatingEntity(Source);
		NetworkSubsystem()->StopReplicatingEntity(Target);
		Source.Destroy();
		Target.Destroy();
		NetworkSubsystem()->ResetClientReplicationForTesting();
		NetworkSubsystem()->EnterClientReplicationModeForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), Shard, SourceCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, SourceCapture.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();
		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), Shard, WithoutPairCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, WithoutPairCapture.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, SourceCapture.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), Shard, TargetCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, TargetCapture.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();
		const FFlecsEntityHandle RemoteTarget = NetworkSubsystem()->FindEntity(TargetCapture.Snapshot.NetworkId);
		const FFlecsEntityHandle RemoteSource = NetworkSubsystem()->FindEntity(SourceCapture.Snapshot.NetworkId);
		ASSERT_THAT(IsFalse(RemoteSource.HasPair<FFlecsReplicationTestValueRelationship>(RemoteTarget)));
	}

	TEST_METHOD(EntityTargetPair_TargetRemovalCancelsPendingFixup)
	{
		const FGuid Shard = FGuid::NewGuid();
		const FFlecsEntityHandle Target = World()->CreateEntity();
		const UE::Flecs::Tests::FCapturedReplicationEntity TargetCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Target);
		const FFlecsEntityHandle Source = World()->CreateEntity()
			.SetPair<FFlecsReplicationTestValueRelationship>(Target,
				FFlecsReplicationTestValueRelationship{ 17 });
		const UE::Flecs::Tests::FCapturedReplicationEntity SourceCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Source);
		NetworkSubsystem()->StopReplicatingEntity(Source);
		NetworkSubsystem()->StopReplicatingEntity(Target);
		Source.Destroy();
		Target.Destroy();
		NetworkSubsystem()->ResetClientReplicationForTesting();
		NetworkSubsystem()->EnterClientReplicationModeForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), Shard, SourceCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, SourceCapture.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();
		UE::Flecs::Tests::EnqueueRemoval(NetworkSubsystem(), Shard, TargetCapture.Snapshot.NetworkId);
		NetworkSubsystem()->FlushClientReplicationForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), Shard, TargetCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, TargetCapture.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();
		const FFlecsEntityHandle RemoteTarget = NetworkSubsystem()->FindEntity(TargetCapture.Snapshot.NetworkId);
		const FFlecsEntityHandle RemoteSource = NetworkSubsystem()->FindEntity(SourceCapture.Snapshot.NetworkId);
		ASSERT_THAT(IsFalse(RemoteSource.HasPair<FFlecsReplicationTestValueRelationship>(RemoteTarget)));
	}

	TEST_METHOD(EntityTargetPair_SourceRemovalCancelsPendingFixup)
	{
		const FGuid Shard = FGuid::NewGuid();
		const FFlecsEntityHandle Target = World()->CreateEntity();
		const UE::Flecs::Tests::FCapturedReplicationEntity TargetCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Target);
		const FFlecsEntityHandle Source = World()->CreateEntity()
			.SetPair<FFlecsReplicationTestValueRelationship>(Target,
				FFlecsReplicationTestValueRelationship{ 17 });
		const UE::Flecs::Tests::FCapturedReplicationEntity SourceCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Source);
		NetworkSubsystem()->StopReplicatingEntity(Source);
		NetworkSubsystem()->StopReplicatingEntity(Target);
		Source.Destroy();
		Target.Destroy();
		NetworkSubsystem()->ResetClientReplicationForTesting();
		NetworkSubsystem()->EnterClientReplicationModeForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), Shard, SourceCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, SourceCapture.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();
		UE::Flecs::Tests::EnqueueRemoval(NetworkSubsystem(), Shard, SourceCapture.Snapshot.NetworkId);
		NetworkSubsystem()->FlushClientReplicationForTesting();

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), Shard, TargetCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, TargetCapture.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();
		ASSERT_THAT(IsFalse(NetworkSubsystem()->FindEntity(SourceCapture.Snapshot.NetworkId).IsValid()));
	}

}; // FlecsReplicationPairFixupTests

#endif
