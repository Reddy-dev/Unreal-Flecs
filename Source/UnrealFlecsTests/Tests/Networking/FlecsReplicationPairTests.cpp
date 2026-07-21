// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "FlecsReplicationTestHelpers.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

FLECS_REPLICATION_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsReplicationPairTests,
	"UnrealFlecs.Networking.Replication.PairsReferences",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter,
	"[Flecs][Networking][Replication]")
{
	TEST_METHOD(PairLayouts_RepresentSchemaStableAndEntityTargets)
	{
		const FFlecsEntityHandle StaticTarget = World()->CreateEntity(TEXT("ReplicationStaticTarget"))
			.Add<FFlecsStablePathTag>();
		
		const FFlecsEntityHandle NetworkTarget = World()->CreateEntity();
		const FFlecsNetworkId TargetId = NetworkSubsystem()->BeginReplicatingEntity(NetworkTarget);
		
		const FFlecsEntityHandle Source = World()->CreateEntity()
			.AddPair<FFlecsReplicationTestRelationship>(StaticTarget)
			.AddPair<FFlecsReplicationTestRelationship>(NetworkTarget);
		
		const UE::Flecs::Tests::FCapturedReplicationEntity Captured =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Source);

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
		const FFlecsEntityHandle SymbolPrimary = World()->CreateEntity()
			.Add<FFlecsReplicatedTrait>();
		SymbolPrimary.GetEntity().set_symbol("ReplicationSymbolPrimary");
		
		const FFlecsEntityHandle SymbolSecondary = World()->CreateEntity();
		SymbolSecondary.GetEntity().set_symbol("ReplicationSymbolSecondary");
		
		const FFlecsEntityHandle PathPrimary = World()->CreateEntity(TEXT("ReplicationPathPrimary"))
			.Add<FFlecsStablePathTag>()
			.Add<FFlecsReplicatedTrait>();
		const FFlecsEntityHandle PathSecondary = World()->CreateEntity(TEXT("ReplicationPathSecondary"))
			.Add<FFlecsStablePathTag>();

		const FFlecsEntityHandle NetworkPrimary = World()->CreateEntity();
		const UE::Flecs::Tests::FCapturedReplicationEntity NetworkPrimaryCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), NetworkPrimary);
		const FFlecsEntityHandle NetworkSecondary = World()->CreateEntity();
		const UE::Flecs::Tests::FCapturedReplicationEntity NetworkSecondaryCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), NetworkSecondary);

		const FFlecsEntityHandle Source = World()->CreateEntity()
			.Add(NetworkPrimary)
			.Add(SymbolPrimary)
			.Add(PathPrimary)
			.AddPair(NetworkPrimary, NetworkSecondary)
			.AddPair(SymbolPrimary, SymbolSecondary)
			.AddPair(PathPrimary, PathSecondary);
		
		const UE::Flecs::Tests::FCapturedReplicationEntity SourceCapture =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Source);

		ASSERT_THAT(AreEqual(6, SourceCapture.Layout.Keys.Num()));
		for (const FFlecsReplicationKey& Key : SourceCapture.Layout.Keys)
		{
			ASSERT_THAT(IsTrue(Key.StorageKind == EFlecsReplicationKeyStorageKind::None));
			ASSERT_THAT(IsNull(Key.TryGetStorageDescriptor(World())));
		}

		NetworkSubsystem()->StopReplicatingEntity(Source);
		NetworkSubsystem()->StopReplicatingEntity(NetworkPrimary);
		NetworkSubsystem()->StopReplicatingEntity(NetworkSecondary);
		Source.Destroy();
		NetworkPrimary.Destroy();
		NetworkSecondary.Destroy();
		NetworkSubsystem()->ResetClientReplicationForTesting();
		NetworkSubsystem()->EnterClientReplicationModeForTesting();
		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), Shard, NetworkPrimaryCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, NetworkPrimaryCapture.Snapshot);
		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), Shard, NetworkSecondaryCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, NetworkSecondaryCapture.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();

		const FFlecsEntityHandle RemoteNetworkPrimary = NetworkSubsystem()->FindEntity(
			NetworkPrimaryCapture.Snapshot.NetworkId);
		const FFlecsEntityHandle RemoteNetworkSecondary = NetworkSubsystem()->FindEntity(
			NetworkSecondaryCapture.Snapshot.NetworkId);
		ASSERT_THAT(IsTrue(RemoteNetworkPrimary.IsValid()));
		ASSERT_THAT(IsTrue(RemoteNetworkSecondary.IsValid()));

		UE::Flecs::Tests::EnqueueLayout(NetworkSubsystem(), Shard, SourceCapture.Layout);
		UE::Flecs::Tests::EnqueueSnapshot(NetworkSubsystem(), Shard, SourceCapture.Snapshot);
		NetworkSubsystem()->FlushClientReplicationForTesting();

		const FFlecsEntityHandle RemoteSource = NetworkSubsystem()->FindEntity(SourceCapture.Snapshot.NetworkId);
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
		const FFlecsEntityHandle WithComponent = World()->GetAlive(
			World()->GetIdIfRegistered<FFlecsReplicationTestWithValue>());
		ASSERT_THAT(IsTrue(WithComponent.HasPair(flecs::With,
			World()->GetIdIfRegistered<FFlecsReplicationTestRequiredTag>())));
		const FFlecsEntityHandle Source = World()->CreateEntity().Set<FFlecsReplicationTestWithValue>({ 11 });
		ASSERT_THAT(IsTrue(Source.Has<FFlecsReplicationTestRequiredTag>()));
		const UE::Flecs::Tests::FCapturedReplicationEntity Initial =
			UE::Flecs::Tests::CaptureEntity(NetworkSubsystem(), CaptureTransport(), Source);
		ASSERT_THAT(AreEqual(2, Initial.Layout.Keys.Num()));

		CaptureTransport()->Snapshots.Reset();
		Source.Add<FFlecsReplicationTestTag>();
		Source.Remove<FFlecsReplicationTestTag>();
		NetworkSubsystem()->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(0, CaptureTransport()->Snapshots.Num()));
	}


}; // FlecsReplicationPairTests

#endif
