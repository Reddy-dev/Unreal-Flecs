// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "UnrealFlecsConfigMacros.h"
#include "UnrealFlecsTests/Fixtures/FlecsReplicationFixture.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Iris/ReplicationSystem/NetObjectFactoryRegistry.h"
#include "UObject/UObjectGlobals.h"

#include "Networking/FlecsNetDirtyTag.h"
#include "Networking/FlecsReplicationProfile.h"
#include "Networking/FlecsReplicationProfileDataAsset.h"
#include "Networking/FlecsReplicationShardSelection.h"
#include "Networking/FlecsReplicationUpdateQueue.h"
#include "Networking/FlecsNetworkingModuleSettings.h"
#include "Networking/FlecsReplicatedEntityComponent.h"
#include "Networking/Layout/FlecsLayoutReplicatorFastArray.h"
#include "Networking/Layout/FlecsReplicationLayoutRegistry.h"
#include "Networking/Shards/FlecsNetEntityTable.h"
#include "Networking/Shards/FlecsNetEntityTableNetFactory.h"
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

	TEST_METHOD(ReplicationQueue_CoalescesByLatestStateRevision)
	{
		FFlecsReplicationUpdateQueue Queue;
		const FFlecsNetworkId NetworkId(21, 1);

		FFlecsEntityReplicationSnapshot OlderSnapshot;
		OlderSnapshot.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		OlderSnapshot.StateRevision = 4;

		FFlecsEntityReplicationSnapshot NewerSnapshot = OlderSnapshot;
		NewerSnapshot.StateRevision = 5;

		Queue.EnqueueSnapshot(NetworkId, NewerSnapshot);
		Queue.EnqueueSnapshot(NetworkId, OlderSnapshot);

		const TArray<FFlecsReplicationQueuedUpdate> Updates = Queue.Drain();
		ASSERT_THAT(AreEqual(1, Updates.Num()));
		if (Updates.Num() != 1)
		{
			return;
		}

		ASSERT_THAT(IsFalse(Updates[0].bRemove));
		ASSERT_THAT(AreEqual(5u, Updates[0].StateRevision));
		ASSERT_THAT(AreEqual(5u, Updates[0].Snapshot.StateRevision));
	}

	TEST_METHOD(ReplicationQueue_OrdersRemovalsByStateRevision)
	{
		FFlecsReplicationUpdateQueue Queue;
		const FFlecsNetworkId NetworkId(22, 1);

		FFlecsEntityReplicationSnapshot Snapshot;
		Snapshot.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		Snapshot.StateRevision = 7;
		FFlecsEntityReplicationSnapshot OlderSnapshot = Snapshot;
		OlderSnapshot.StateRevision = 6;

		Queue.EnqueueSnapshot(NetworkId, Snapshot);
		Queue.EnqueueRemoval(NetworkId, 7);
		Queue.EnqueueSnapshot(NetworkId, OlderSnapshot);

		const TArray<FFlecsReplicationQueuedUpdate> Updates = Queue.Drain();
		ASSERT_THAT(AreEqual(1, Updates.Num()));
		if (Updates.Num() != 1)
		{
			return;
		}

		ASSERT_THAT(IsTrue(Updates[0].bRemove));
		ASSERT_THAT(AreEqual(7u, Updates[0].StateRevision));
	}

	TEST_METHOD(ReplicationProfile_AssetCreatesFlecsPrefab)
	{
		UFlecsReplicationProfileDataAsset* Asset = NewObject<UFlecsReplicationProfileDataAsset>(NetworkSubsystem());
		Asset->ProfileName = FName(TEXT("TestProfile"));
		Asset->Definition.FilterName = FName(TEXT("TestFilter"));
		Asset->Definition.ObjectPrioritizerName = FName(TEXT("TestPrioritizer"));
		Asset->Definition.ShardSelectorName = FName(TEXT("Proxy"));

		const FFlecsEntityHandle ProfilePrefab = NetworkSubsystem()->RegisterReplicationProfileAsset(Asset);
		ASSERT_THAT(IsTrue(ProfilePrefab.IsValid()));
		ASSERT_THAT(IsTrue(ProfilePrefab.IsPrefab()));
		ASSERT_THAT(IsTrue(ProfilePrefab.Has<FFlecsReplicationProfileTag>()));

		const FFlecsEntityHandle Entity = World()->CreateEntity().AddPrefab(ProfilePrefab.GetFlecsId());
		FFlecsReplicationProfile ResolvedProfile;
		ASSERT_THAT(IsTrue(NetworkSubsystem()->ResolveReplicationProfile(Entity, ResolvedProfile)));
		ASSERT_THAT(IsTrue(ResolvedProfile == Asset->Definition));
	}

	TEST_METHOD(ReplicationProfile_SetProfileReplacesRegisteredIsAProfile)
	{
		FFlecsReplicationProfile FirstDefinition;
		FirstDefinition.FilterName = FName(TEXT("FirstFilter"));
		const FFlecsEntityHandle FirstPrefab = NetworkSubsystem()->RegisterReplicationProfileDefinition(
			FName(TEXT("FirstProfile")), FirstDefinition);

		FFlecsReplicationProfile SecondDefinition;
		SecondDefinition.FilterName = FName(TEXT("SecondFilter"));
		const FFlecsEntityHandle SecondPrefab = NetworkSubsystem()->RegisterReplicationProfileDefinition(
			FName(TEXT("SecondProfile")), SecondDefinition);

		const FFlecsEntityHandle Entity = World()->CreateEntity()
			.Add<FFlecsReplicatedEntityComponent>()
			.AddPrefab(FirstPrefab.GetFlecsId());
		ASSERT_THAT(IsTrue(NetworkSubsystem()->SetReplicationProfile(Entity, SecondPrefab)));

		const FFlecsReplicatedEntityComponent* ReplicatedEntity = Entity.TryGet<FFlecsReplicatedEntityComponent>();
		ASSERT_THAT(IsNotNull(ReplicatedEntity));
		if (!ReplicatedEntity)
		{
			return;
		}

		ASSERT_THAT(IsTrue(ReplicatedEntity->ProfileId == FName(TEXT("SecondProfile"))));

		FFlecsReplicationProfile ResolvedProfile;
		ASSERT_THAT(IsTrue(NetworkSubsystem()->ResolveReplicationProfile(Entity, ResolvedProfile)));
		ASSERT_THAT(IsTrue(ResolvedProfile == SecondDefinition));
	}

	TEST_METHOD(ReplicationShardSelector_UsesRegisteredImplementation)
	{
		bool bSelectorCalled = false;
		ASSERT_THAT(IsTrue(NetworkSubsystem()->RegisterReplicationShardSelector(
			FName(TEXT("TestSelector")),
			[&bSelectorCalled](const FFlecsEntityHandle&, const FFlecsNetworkId&,
				const FFlecsReplicationProfile&, FFlecsReplicationShardSelection& OutSelection)
			{
				bSelectorCalled = true;
				OutSelection.ShardClass = UFlecsNetEntityProxy::StaticClass();
				OutSelection.ShardGroupKey = FName(TEXT("TestShardGroup"));
				return true;
			})));

		FFlecsReplicationProfile Profile;
		Profile.ShardSelectorName = FName(TEXT("TestSelector"));
		FFlecsReplicationShardSelection Selection;
		const FFlecsNetworkId NetworkId(44, 1);

		ASSERT_THAT(IsTrue(NetworkSubsystem()->SelectReplicationShard(
			World()->CreateEntity(), NetworkId, Profile, Selection)));
		ASSERT_THAT(IsTrue(bSelectorCalled));
		ASSERT_THAT(IsTrue(Selection.ShardClass == UFlecsNetEntityProxy::StaticClass()));
		ASSERT_THAT(IsTrue(Selection.ShardGroupKey == FName(TEXT("TestShardGroup"))));
	}

	TEST_METHOD(EntityTableAndEntityProxy_AreShardStorageTypes)
	{
		ASSERT_THAT(IsTrue(
			UFlecsNetEntityTable::StaticClass()->IsChildOf(UFlecsNetShardBase::StaticClass())));
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

	TEST_METHOD(EntityProxy_UsesAssignedWorldWithoutWorldOuter)
	{
		UFlecsNetEntityProxy* Proxy = NewObject<UFlecsNetEntityProxy>(GetTransientPackage());
		ASSERT_THAT(IsTrue(Proxy->GetOuter() == GetTransientPackage()));

		Proxy->SetOwningWorld(NetworkSubsystem()->GetWorld());
		ASSERT_THAT(IsTrue(Proxy->GetWorld() == NetworkSubsystem()->GetWorld()));
	}

	TEST_METHOD(EntityProxy_QueuesUpdateUntilWorldIsAssigned)
	{
		UFlecsNetEntityProxy* Proxy = NewObject<UFlecsNetEntityProxy>(GetTransientPackage());
		const FFlecsNetworkId NetworkId(16, 3);

		FFlecsEntityReplicationSnapshot Snapshot;
		Snapshot.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		Snapshot.StateRevision = 1;

		Proxy->NetworkId = NetworkId;
		Proxy->Snapshot = Snapshot;
		Proxy->OnRep_Snapshot();
		ASSERT_THAT(AreEqual(0, NetworkSubsystem()->GetQueuedReplicationUpdateCount()));

		Proxy->SetOwningWorld(NetworkSubsystem()->GetWorld());
		ASSERT_THAT(AreEqual(1, NetworkSubsystem()->GetQueuedReplicationUpdateCount()));
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

	TEST_METHOD(EntityProxy_RemovingEntityDoesNotRequireShardTeardown)
	{
		UFlecsNetEntityProxy* Proxy = NewObject<UFlecsNetEntityProxy>(NetworkSubsystem());
		const FFlecsNetworkId NetworkId(18, 3);

		FFlecsEntityReplicationSnapshot Snapshot;
		Snapshot.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		Snapshot.StateRevision = 1;

		Proxy->PublishNetEntity(NetworkId, Snapshot);
		ASSERT_THAT(IsFalse(Proxy->IsEmpty()));
		ASSERT_THAT(IsTrue(Proxy->CanAcceptNetEntity(NetworkId, Snapshot)));
		ASSERT_THAT(IsFalse(Proxy->CanAcceptNetEntity(FFlecsNetworkId(19, 3), Snapshot)));

		Proxy->RemoveNetEntity(NetworkId);
		ASSERT_THAT(IsTrue(Proxy->IsEmpty()));
	}

	TEST_METHOD(EntityTable_UsesRegisteredFactoryAndUpsertsByNetworkId)
	{
		UFlecsNetEntityTable* Table = NewObject<UFlecsNetEntityTable>(NetworkSubsystem());
		UE::Net::FRootObjectSettings Settings;
		Table->ConfigureObjectSettings(Settings);

		ASSERT_THAT(IsTrue(Settings.FactoryName == UFlecsNetEntityTableNetFactory::GetFactoryName()));
		ASSERT_THAT(IsTrue(
			UE::Net::FNetObjectFactoryRegistry::GetFactoryIdFromName(Settings.FactoryName)
				!= UE::Net::InvalidNetObjectFactoryId));

		const FFlecsNetworkId NetworkId(20, 3);
		FFlecsEntityReplicationSnapshot InitialSnapshot;
		InitialSnapshot.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		InitialSnapshot.StateRevision = 1;

		Table->PublishNetEntity(NetworkId, InitialSnapshot);
		ASSERT_THAT(AreEqual(1, Table->EntityTable.Items.Num()));
		ASSERT_THAT(IsFalse(Table->IsEmpty()));

		FFlecsEntityReplicationSnapshot UpdatedSnapshot = InitialSnapshot;
		UpdatedSnapshot.StateRevision = 2;
		Table->PublishNetEntity(NetworkId, UpdatedSnapshot);
		ASSERT_THAT(AreEqual(1, Table->EntityTable.Items.Num()));
		ASSERT_THAT(AreEqual(2u, Table->EntityTable.Items[0].Snapshot.StateRevision));

		Table->RemoveNetEntity(NetworkId);
		ASSERT_THAT(IsTrue(Table->IsEmpty()));
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
		NetworkSubsystem()->ApplyQueuedReplicationUpdates();

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
		NetworkSubsystem()->ApplyQueuedReplicationUpdates();
		ASSERT_THAT(AreEqual(91, ReceivedEntity.GetValue().Get<FFlecsReplicationTestValue>().Value));

		Proxy->Snapshot = InitialSnapshot;
		Proxy->OnRep_Snapshot();
		NetworkSubsystem()->ApplyQueuedReplicationUpdates();
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
