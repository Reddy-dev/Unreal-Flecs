// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "CQTest.h"
#include "Iris/ReplicationSystem/NetObjectFactoryRegistry.h"
#include "Networking/FlecsIrisReplicationShard.h"
#include "Networking/FlecsIrisReplicationTransport.h"
#include "Networking/FlecsIrisShardObjectFactory.h"

TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsIrisReplicationTransportTests,
	"UnrealFlecs.Networking.Replication.Iris",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter,
	"[Flecs][Networking][Replication][Iris]")
{
	TEST_METHOD(ProviderAndFactory_AreRegistered)
	{
		ASSERT_THAT(IsTrue(FFlecsReplicationTransportRegistry::FindProvider(TEXT("Iris"))
			== UFlecsIrisReplicationTransport::StaticClass()));
		ASSERT_THAT(IsTrue(UE::Net::FNetObjectFactoryRegistry::IsValidFactoryId(
			UE::Net::FNetObjectFactoryRegistry::GetFactoryIdFromName(
				UFlecsIrisShardObjectFactory::GetFactoryName()))));
	}

	TEST_METHOD(ShardFastArrays_UpsertAndRemoveCompleteEntitySnapshots)
	{
		UFlecsIrisReplicationShard* Shard = NewObject<UFlecsIrisReplicationShard>();
		Shard->InitializeServer(nullptr, FFlecsReplicationRouteKey::Default(), 20.0f, 1.0f);

		FFlecsReplicationLayoutDefinition Layout;
		Layout.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		Shard->UpsertLayout(Layout);
		Shard->UpsertLayout(Layout);
		ASSERT_THAT(AreEqual(1, Shard->GetLayoutCount()));

		FFlecsReplicatedEntitySnapshot Snapshot;
		Snapshot.NetworkId = FFlecsNetworkId(5, 1, 9);
		Snapshot.LayoutId = Layout.LayoutId;
		Snapshot.StateRevision = 1;
		Shard->UpsertEntity(Snapshot);
		Snapshot.StateRevision = 2;
		Shard->UpsertEntity(Snapshot);
		ASSERT_THAT(AreEqual(1, Shard->GetEntityCount()));

		Shard->RemoveEntity(Snapshot.NetworkId);
		ASSERT_THAT(AreEqual(0, Shard->GetEntityCount()));
		Shard->StopReplication();
	}
};

#endif
