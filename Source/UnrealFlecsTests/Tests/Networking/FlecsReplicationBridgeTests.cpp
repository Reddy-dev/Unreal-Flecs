// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "UnrealFlecsConfigMacros.h"
#include "UnrealFlecsTests/Fixtures/FlecsReplicationFixture.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Iris/ReplicationSystem/NetObjectFactoryRegistry.h"

#include "Networking/FlecsNetDirtyTag.h"
#include "Networking/FlecsReplicationInbox.h"
#include "Networking/FlecsNetworkingModuleSettings.h"
#include "Networking/FlecsReplicatedEntityComponent.h"
#include "Networking/Layout/FlecsLayoutReplicatorFastArray.h"
#include "Networking/Layout/FlecsReplicationLayoutRegistry.h"
#include "Networking/Shards/FlecsNetEntityPage.h"
#include "Networking/Shards/FlecsNetEntityProxy.h"
#include "Networking/Shards/FlecsNetEntityProxyNetFactory.h"
#include "Networking/Shards/FlecsNetShardBase.h"

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

	TEST_METHOD(ReplicationInbox_IsInstalledAsSingleton)
	{
		ASSERT_THAT(IsTrue(World()->Has<FFlecsReplicationInbox>()));
	}

	TEST_METHOD(ReplicationInbox_CoalescesByLatestStateRevision)
	{
		FFlecsReplicationInbox Inbox;
		const FFlecsNetworkId NetworkId(21, 1);

		FFlecsEntityReplicationSnapshot OlderSnapshot;
		OlderSnapshot.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		OlderSnapshot.StateRevision = 4;

		FFlecsEntityReplicationSnapshot NewerSnapshot = OlderSnapshot;
		NewerSnapshot.StateRevision = 5;

		Inbox.EnqueueSnapshot(NetworkId, NewerSnapshot);
		Inbox.EnqueueSnapshot(NetworkId, OlderSnapshot);

		const TArray<FFlecsReplicationInboxUpdate> Updates = Inbox.Drain();
		ASSERT_THAT(AreEqual(1, Updates.Num()));
		if (Updates.Num() != 1)
		{
			return;
		}

		ASSERT_THAT(IsFalse(Updates[0].bRemove));
		ASSERT_THAT(AreEqual(5u, Updates[0].StateRevision));
		ASSERT_THAT(AreEqual(5u, Updates[0].Snapshot.StateRevision));
	}

	TEST_METHOD(ReplicationInbox_OrdersRemovalsByStateRevision)
	{
		FFlecsReplicationInbox Inbox;
		const FFlecsNetworkId NetworkId(22, 1);

		FFlecsEntityReplicationSnapshot Snapshot;
		Snapshot.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		Snapshot.StateRevision = 7;
		FFlecsEntityReplicationSnapshot OlderSnapshot = Snapshot;
		OlderSnapshot.StateRevision = 6;

		Inbox.EnqueueSnapshot(NetworkId, Snapshot);
		Inbox.EnqueueRemoval(NetworkId, 7);
		Inbox.EnqueueSnapshot(NetworkId, OlderSnapshot);

		const TArray<FFlecsReplicationInboxUpdate> Updates = Inbox.Drain();
		ASSERT_THAT(AreEqual(1, Updates.Num()));
		if (Updates.Num() != 1)
		{
			return;
		}

		ASSERT_THAT(IsTrue(Updates[0].bRemove));
		ASSERT_THAT(AreEqual(7u, Updates[0].StateRevision));
	}

	TEST_METHOD(EntityPageAndEntityProxy_AreShardStorageTypes)
	{
		ASSERT_THAT(IsTrue(
			UFlecsNetEntityPage::StaticClass()->IsChildOf(UFlecsNetShardBase::StaticClass())));
		ASSERT_THAT(IsTrue(
			UFlecsNetEntityProxy::StaticClass()->IsChildOf(UFlecsNetShardBase::StaticClass())));
	}

	TEST_METHOD(EntityProxy_UsesAlwaysRelevantRegisteredFactory)
	{
		UFlecsNetEntityProxy* Proxy = NewObject<UFlecsNetEntityProxy>(NetworkSubsystem());
		UE::Net::FRootObjectSettings Settings;
		Proxy->ConfigureObjectSettings(Settings);

		ASSERT_THAT(IsTrue(Settings.bIsAlwaysRelevant));
		ASSERT_THAT(IsFalse(Settings.bIsNotRouted));
		ASSERT_THAT(IsTrue(
			Settings.FactoryName == UFlecsNetEntityProxyNetFactory::GetFactoryName()));
		ASSERT_THAT(IsTrue(
			UE::Net::FNetObjectFactoryRegistry::GetFactoryIdFromName(Settings.FactoryName)
				!= UE::Net::InvalidNetObjectFactoryId));
	}

	TEST_METHOD(EntityProxy_PublishNetEntity_CopiesIdentityAndSnapshot)
	{
		UFlecsNetEntityProxy* Proxy = NewObject<UFlecsNetEntityProxy>(NetworkSubsystem());
		const FFlecsNetworkId NetworkId(17, 3);

		FFlecsEntityReplicationSnapshot Snapshot;
		Snapshot.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		Snapshot.StateRevision = 9;

		FFlecsReplicatedValue Value;
		Value.KeyIndex = 2;
		Value.Bytes = { 4, 8, 15, 16, 23, 42 };
		Snapshot.Values.Add(Value);

		Proxy->PublishNetEntity(NetworkId, Snapshot);

		ASSERT_THAT(IsTrue(Proxy->NetworkId == NetworkId));
		ASSERT_THAT(IsTrue(Proxy->Snapshot.LayoutId == Snapshot.LayoutId));
		ASSERT_THAT(AreEqual(Snapshot.StateRevision, Proxy->Snapshot.StateRevision));
		ASSERT_THAT(AreEqual(Snapshot.Values.Num(), Proxy->Snapshot.Values.Num()));
		ASSERT_THAT(IsTrue(Proxy->Snapshot.Values[0].Bytes == Value.Bytes));
	}

	TEST_METHOD(EntityProxy_AppliesUpdatesAndRejectsStaleRevisions)
	{
		const FFlecsEntityHandle SourceEntity = World()->CreateEntity()
			.Set<FFlecsReplicationTestValue>({ 17 });

		bool bCreatedNewLayout = false;
		const TValueOrError<const FFlecsReplicationLayoutDefinition*, FString> LayoutResult =
			NetworkSubsystem()->GetLayoutRegistry().BuildForEntity(
				World(), SourceEntity, bCreatedNewLayout);

		ASSERT_THAT(IsFalse(LayoutResult.HasError()));
		if (LayoutResult.HasError())
		{
			return;
		}

		const FFlecsReplicationLayoutDefinition* LayoutDefinition = LayoutResult.GetValue();
		ASSERT_THAT(IsNotNull(LayoutDefinition));
		if (!LayoutDefinition)
		{
			return;
		}

		FFlecsEntityReplicationSnapshot InitialSnapshot;
		InitialSnapshot.LayoutId = LayoutDefinition->LayoutId;
		InitialSnapshot.FillFromEntity(SourceEntity, NetworkSubsystem()->GetLayoutRegistry());

		const FFlecsNetworkId NetworkId(31, 1);
		UFlecsNetEntityProxy* Proxy = NewObject<UFlecsNetEntityProxy>(NetworkSubsystem());
		Proxy->SetOwningNetworkWorldSubsystem(NetworkSubsystem());
		Proxy->NetworkId = NetworkId;
		Proxy->Snapshot = InitialSnapshot;
		Proxy->OnRep_Snapshot();
		World()->Progress(0.0);

		TOptional<FFlecsEntityHandle> ReceivedEntity = NetworkSubsystem()->GetEntityFromNetworkId(NetworkId);
		ASSERT_THAT(IsTrue(ReceivedEntity.IsSet()));
		if (!ReceivedEntity.IsSet())
		{
			return;
		}

		ASSERT_THAT(AreEqual(17, ReceivedEntity.GetValue().Get<FFlecsReplicationTestValue>().Value));

		SourceEntity.Set<FFlecsReplicationTestValue>({ 91 });
		FFlecsEntityReplicationSnapshot UpdatedSnapshot = InitialSnapshot;
		UpdatedSnapshot.FillFromEntity(SourceEntity, NetworkSubsystem()->GetLayoutRegistry());

		Proxy->Snapshot = UpdatedSnapshot;
		Proxy->OnRep_Snapshot();
		World()->Progress(0.0);
		ASSERT_THAT(AreEqual(91, ReceivedEntity.GetValue().Get<FFlecsReplicationTestValue>().Value));

		Proxy->Snapshot = InitialSnapshot;
		Proxy->OnRep_Snapshot();
		World()->Progress(0.0);
		ASSERT_THAT(AreEqual(91, ReceivedEntity.GetValue().Get<FFlecsReplicationTestValue>().Value));
	}

	TEST_METHOD(LayoutFastArray_AddsIdempotently)
	{
		FFlecsReplicationLayoutDefinition Layout;
		Layout.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());

		FFlecsReplicatorFastArray Layouts;

		ASSERT_THAT(IsTrue(Layouts.AddLayout(Layout)));
		ASSERT_THAT(IsFalse(Layouts.AddLayout(Layout)));
		ASSERT_THAT(IsTrue(Layouts.Items.Num() == 1));
		ASSERT_THAT(IsTrue(Layouts.Items[0].ReplicationID != INDEX_NONE));
		ASSERT_THAT(IsTrue(Layouts.FindLayout(Layout.LayoutId) != nullptr));
	}

	
}; // FlecsReplicationBridgeTests

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS
