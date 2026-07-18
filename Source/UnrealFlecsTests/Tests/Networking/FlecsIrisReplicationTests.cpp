// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "CQTest.h"
#include "Iris/ReplicationSystem/NetObjectFactoryRegistry.h"
#include "Networking/FlecsIrisReplicationShard.h"
#include "Networking/FlecsIrisReplicationTransport.h"
#include "Networking/FlecsIrisShardObjectFactory.h"
#include "Networking/FlecsNetworkWorldSubsystem.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

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

	TEST_METHOD(PageRetainsFullBaseline_AndReclaimsUnusedLayouts)
	{
		UFlecsIrisReplicationShard* Shard = NewObject<UFlecsIrisReplicationShard>();
		const FFlecsReplicationRouteDescriptor Route = FFlecsReplicationRouteDescriptor::Default();
		Shard->InitializeServer(nullptr, Route, 0);
		ASSERT_THAT(IsTrue(Shard->GetSourceShardId().IsValid()));

		FFlecsReplicationLayoutDefinition Layout;
		Layout.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		Shard->UpsertLayout(Layout);
		Shard->UpsertLayout(Layout);
		ASSERT_THAT(AreEqual(1, Shard->GetLayoutCount()));

		FFlecsReplicatedEntityUpdate Update;
		Update.NetworkId = FFlecsNetworkId(5, 1, 9);
		Update.LayoutId = Layout.LayoutId;
		Update.Route = Route;
		Update.StateRevision = 1;
		Shard->UpsertEntity(Update);
		Update.StateRevision = 2;
		Update.Kind = EFlecsReplicatedEntityUpdateKind::Delta;
		Shard->UpsertEntity(Update);
		ASSERT_THAT(AreEqual(1, Shard->GetEntityCount()));
		ASSERT_THAT(IsNotNull(Shard->FindMaterializedEntity(Update.NetworkId)));

		Shard->RemoveEntity(Update.NetworkId);
		ASSERT_THAT(AreEqual(0, Shard->GetEntityCount()));
		ASSERT_THAT(AreEqual(0, Shard->GetLayoutCount()));
		Shard->StopReplication();
	}
};

TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsIrisReplicationPagingTests,
	"UnrealFlecs.Networking.Replication.Iris.Paging",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter,
	"[Flecs][Networking][Replication][Iris]")
{
	inline static TUniquePtr<FFlecsTestFixtureRAII> Fixture;
	inline static TObjectPtr<UFlecsNetworkWorldSubsystem> NetworkSubsystem = nullptr;
	inline static TObjectPtr<UFlecsIrisReplicationTransport> Transport = nullptr;

	BEFORE_EACH()
	{
		Fixture = MakeUnique<FFlecsTestFixtureRAII>();
		NetworkSubsystem = Fixture->Fixture.GetFlecsWorld()->GetWorld()
			->GetSubsystem<UFlecsNetworkWorldSubsystem>();
		Transport = NewObject<UFlecsIrisReplicationTransport>(NetworkSubsystem);
		NetworkSubsystem->SetReplicationTransportForTesting(Transport);
	}

	AFTER_EACH()
	{
		if (Transport)
		{
			Transport->ShutdownTransport();
		}
		Transport = nullptr;
		NetworkSubsystem = nullptr;
		Fixture.Reset();
	}

	TEST_METHOD(EntityAndByteCaps_CreateStablePages_AndEmptyPagesRetire)
	{
		FFlecsReplicationRouteDescriptor Route;
		Route.LogicalKey = FFlecsReplicationRouteKey(FName(TEXT("Paged")));
		Route.PageEntityLimit = 1;
		Route.PageByteLimit = 64;
		FFlecsReplicationLayoutDefinition Layout;
		Layout.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		Transport->PublishLayout(Route, Layout);

		FFlecsReplicatedEntityUpdate First;
		First.NetworkId = FFlecsNetworkId(1, 1, 1);
		First.StateRevision = 1;
		First.LayoutId = Layout.LayoutId;
		First.Route = Route;
		FFlecsReplicatedValue& FirstValue = First.Values.AddDefaulted_GetRef();
		FirstValue.KeyIndex = 0;
		FirstValue.Bytes.SetNumZeroed(40);
		First.SetKeyChanged(0);
		Transport->PublishEntity(Route, First);
		UFlecsIrisReplicationShard* StableFirstPage = Transport->FindEntityPage(First.NetworkId);
		ASSERT_THAT(IsNotNull(StableFirstPage));

		FFlecsReplicatedEntityUpdate Second = First;
		Second.NetworkId = FFlecsNetworkId(2, 1, 1);
		Transport->PublishEntity(Route, Second);
		ASSERT_THAT(AreEqual(2, Transport->GetPageCount(FName(TEXT("Paged")))));
		ASSERT_THAT(IsTrue(StableFirstPage != Transport->FindEntityPage(Second.NetworkId)));
		ASSERT_THAT(IsTrue(StableFirstPage->GetSourceShardId()
			!= Transport->FindEntityPage(Second.NetworkId)->GetSourceShardId()));

		First.StateRevision = 2;
		First.Kind = EFlecsReplicatedEntityUpdateKind::Delta;
		Transport->PublishEntity(Route, First);
		ASSERT_THAT(IsTrue(StableFirstPage == Transport->FindEntityPage(First.NetworkId)));

		Transport->RemoveEntity(Route, First.NetworkId);
		Transport->TickTransport();
		ASSERT_THAT(AreEqual(2, Transport->GetPageCount(FName(TEXT("Paged")))));
		Transport->TickTransport();
		ASSERT_THAT(AreEqual(1, Transport->GetPageCount(FName(TEXT("Paged")))));

		FFlecsReplicationRouteDescriptor ByteRoute;
		ByteRoute.LogicalKey = FFlecsReplicationRouteKey(FName(TEXT("BytePaged")));
		ByteRoute.PageEntityLimit = 10;
		ByteRoute.PageByteLimit = 64;
		Transport->PublishLayout(ByteRoute, Layout);
		First.NetworkId = FFlecsNetworkId(3, 1, 1);
		First.StateRevision = 1;
		First.Kind = EFlecsReplicatedEntityUpdateKind::Full;
		First.Route = ByteRoute;
		Second = First;
		Second.NetworkId = FFlecsNetworkId(4, 1, 1);
		Transport->PublishEntity(ByteRoute, First);
		Transport->PublishEntity(ByteRoute, Second);
		ASSERT_THAT(AreEqual(2, Transport->GetPageCount(FName(TEXT("BytePaged")))));
	}

	TEST_METHOD(DefaultEntityCap_257EntitiesCreateTwoStablePages)
	{
		FFlecsReplicationRouteDescriptor Route;
		Route.LogicalKey = FFlecsReplicationRouteKey(FName(TEXT("DefaultCap")));
		Route.PageEntityLimit = 256;
		Route.PageByteLimit = 256u * 1024u;
		FFlecsReplicationLayoutDefinition Layout;
		Layout.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		Transport->PublishLayout(Route, Layout);

		UFlecsIrisReplicationShard* FirstPage = nullptr;
		for (uint32 Index = 1; Index <= 257; ++Index)
		{
			FFlecsReplicatedEntityUpdate Update;
			Update.NetworkId = FFlecsNetworkId(Index, 1, 1);
			Update.StateRevision = 1;
			Update.LayoutId = Layout.LayoutId;
			Update.Route = Route;
			Transport->PublishEntity(Route, Update);
			if (Index == 1)
			{
				FirstPage = Transport->FindEntityPage(Update.NetworkId);
			}
		}
		ASSERT_THAT(AreEqual(2, Transport->GetPageCount(Route.LogicalKey.Name)));
		ASSERT_THAT(IsNotNull(FirstPage));
		ASSERT_THAT(IsTrue(FirstPage == Transport->FindEntityPage(FFlecsNetworkId(1, 1, 1))));
	}

	TEST_METHOD(ExplicitRouteMigration_PublishesNewSourceBeforeOldPageRemoval)
	{
		FFlecsReplicationRouteDescriptor OldRoute;
		OldRoute.LogicalKey = FFlecsReplicationRouteKey(FName(TEXT("Old")));
		FFlecsReplicationRouteDescriptor NewRoute;
		NewRoute.LogicalKey = FFlecsReplicationRouteKey(FName(TEXT("New")));
		FFlecsReplicationLayoutDefinition Layout;
		Layout.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		Transport->PublishLayout(OldRoute, Layout);
		FFlecsReplicatedEntityUpdate Update;
		Update.NetworkId = FFlecsNetworkId(1, 1, 1);
		Update.StateRevision = 1;
		Update.LayoutId = Layout.LayoutId;
		Update.Route = OldRoute;
		Transport->PublishEntity(OldRoute, Update);
		UFlecsIrisReplicationShard* OldPage = Transport->FindEntityPage(Update.NetworkId);

		Update.StateRevision = 2;
		Update.Route = NewRoute;
		Transport->MigrateEntity(OldRoute, NewRoute, Layout, Update);
		UFlecsIrisReplicationShard* NewPage = Transport->FindEntityPage(Update.NetworkId);
		ASSERT_THAT(IsNotNull(NewPage));
		ASSERT_THAT(IsTrue(NewPage != OldPage));
		ASSERT_THAT(AreEqual(0, OldPage->GetEntityCount()));
		ASSERT_THAT(AreEqual(1, NewPage->GetEntityCount()));
	}
};

#endif
