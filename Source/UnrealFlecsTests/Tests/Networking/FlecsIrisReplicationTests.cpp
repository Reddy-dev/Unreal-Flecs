// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "CQTest.h"
#include "Iris/ReplicationSystem/NetObjectFactoryRegistry.h"
#include "Networking/FlecsIrisReplicationShard.h"
#include "Networking/FlecsIrisReplicationTransport.h"
#include "Networking/FlecsIrisShardObjectFactory.h"
#include "Networking/FlecsNetworkWorldSubsystem.h"

namespace UE::Flecs::Tests
{
	class FBoundedTestRouter final : public IFlecsReplicationRouter
	{
	public:
		virtual FFlecsReplicationRouteDescriptor Route(const FFlecsEntityHandle&) const override
		{
			FFlecsReplicationRouteDescriptor Result = FFlecsReplicationRouteDescriptor::Default();
			Result.PageEntityLimit = 2;
			Result.PageRetainedPayloadByteLimit = 8;
			return Result;
		}
	}; // class FBoundedTestRouter

	template <typename T>
	void RegisterIrisTestComponent(UFlecsWorld* World)
	{
		const FFlecsComponentHandle Component = World->RegisterComponentType<T>();
		const FFlecsComponentPropertiesDefinition Properties = FFlecsComponentPropertiesDefinition::Make<T>();
		Properties.PropertiesFunction(World, Component, Properties);
	}
}

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsIrisReplicationTransportTests,
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

	TEST_METHOD(ShardFastArrays_UpsertAndRemoveMaterializedEntityState)
	{
		UFlecsIrisReplicationShard* Shard = NewObject<UFlecsIrisReplicationShard>();
		Shard->InitializeServer(nullptr, FFlecsReplicationRouteDescriptor::Default(), 20.0f, 1.0f, 256, 256 * 1024);

		FFlecsReplicationLayoutDefinition Layout;
		Layout.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		Shard->UpsertLayout(Layout);
		Shard->UpsertLayout(Layout);
		ASSERT_THAT(AreEqual(1, Shard->GetLayoutCount()));

		FFlecsReplicatedEntityUpdate Snapshot;
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

	TEST_METHOD(Pages_EnforceCountBytesAllowOversizeAndUseUniqueSources)
	{
		UFlecsIrisReplicationShard* First = NewObject<UFlecsIrisReplicationShard>();
		UFlecsIrisReplicationShard* Second = NewObject<UFlecsIrisReplicationShard>();
		First->InitializeServer(nullptr, FFlecsReplicationRouteDescriptor::Default(), 20.0f, 1.0f, 1, 4);
		Second->InitializeServer(nullptr, FFlecsReplicationRouteDescriptor::Default(), 20.0f, 1.0f, 1, 4);
		ASSERT_THAT(IsTrue(First->GetSourceShardId().IsValid()));
		ASSERT_THAT(IsTrue(First->GetSourceShardId() != Second->GetSourceShardId()));

		FFlecsReplicatedEntityUpdate Oversize;
		Oversize.NetworkId = FFlecsNetworkId(1, 1, 1);
		Oversize.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		Oversize.StateRevision = 1;
		Oversize.ChangedKeys.Add(0);
		FFlecsReplicatedValue& OversizeValue = Oversize.Values.AddDefaulted_GetRef();
		OversizeValue.KeyIndex = 0;
		OversizeValue.Bytes = { 1, 2, 3, 4, 5 };
		ASSERT_THAT(IsTrue(First->CanFitUpdate(Oversize)));
		First->UpsertEntity(Oversize);
		Oversize.StateRevision++;
		Oversize.Values[0].Bytes.Add(6);
		ASSERT_THAT(IsTrue(First->CanFitUpdate(Oversize)));

		FFlecsReplicatedEntityUpdate Another = Oversize;
		Another.NetworkId = FFlecsNetworkId(2, 1, 1);
		ASSERT_THAT(IsFalse(First->CanFitUpdate(Another)));
		First->StopReplication();
		Second->StopReplication();
	}

	TEST_METHOD(Transport_UsesStableFirstFitMigratesGrowthAndRetiresEmptyPages)
	{
		UFlecsWorld* TestWorld = World();
		UE::Flecs::Tests::RegisterIrisTestComponent<FFlecsReplicationTestNativeValue>(TestWorld);
		UE::Flecs::Tests::RegisterIrisTestComponent<FFlecsReplicationTestRequiredTag>(TestWorld);
		UE::Flecs::Tests::RegisterIrisTestComponent<FFlecsReplicationTestWithValue>(TestWorld);
		UFlecsNetworkWorldSubsystem* Subsystem =
			TestWorld->GetWorld()->GetSubsystem<UFlecsNetworkWorldSubsystem>();
		UFlecsIrisReplicationTransport* Transport = NewObject<UFlecsIrisReplicationTransport>(Subsystem);
		Subsystem->SetReplicationTransportForTesting(Transport);
		Subsystem->SetReplicationRouter(MakeUnique<UE::Flecs::Tests::FBoundedTestRouter>());

		const FFlecsEntityHandle First = TestWorld->CreateEntity().Set<FFlecsReplicationTestNativeValue>({ 1 });
		const FFlecsEntityHandle Second = TestWorld->CreateEntity().Set<FFlecsReplicationTestNativeValue>({ 2 });
		const FFlecsNetworkId FirstId = Subsystem->BeginReplicatingEntity(First);
		const FFlecsNetworkId SecondId = Subsystem->BeginReplicatingEntity(Second);
		Subsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(1, Transport->GetPageCountForTesting()));
		const UFlecsIrisReplicationShard* StablePage = Transport->GetEntityPageForTesting(FirstId);
		ASSERT_THAT(IsTrue(StablePage == Transport->GetEntityPageForTesting(SecondId)));

		First.Set<FFlecsReplicationTestNativeValue>({ 3 });
		Subsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(IsTrue(StablePage == Transport->GetEntityPageForTesting(FirstId)));

		First.Set<FFlecsReplicationTestWithValue>({ 4 });
		Subsystem->FlushServerReplicationForTesting();
		ASSERT_THAT(AreEqual(2, Transport->GetPageCountForTesting()));
		ASSERT_THAT(IsTrue(StablePage != Transport->GetEntityPageForTesting(FirstId)));
		const UFlecsIrisReplicationShard* MigratedPage = Transport->GetEntityPageForTesting(FirstId);
		ASSERT_THAT(IsTrue(MigratedPage != nullptr));
		const FFlecsReplicatedEntityUpdate* MigratedHeader = MigratedPage->FindHeaderForTesting(FirstId);
		ASSERT_THAT(IsTrue(MigratedHeader != nullptr));
		ASSERT_THAT(IsTrue(MigratedHeader->Kind
			== EFlecsReplicatedEntityUpdateKind::Full));

		Subsystem->StopReplicatingEntity(FirstId);
		ASSERT_THAT(AreEqual(2, Transport->GetPageCountForTesting()));
		Transport->TickTransport();
		ASSERT_THAT(AreEqual(2, Transport->GetPageCountForTesting()));
		Transport->TickTransport();
		ASSERT_THAT(AreEqual(1, Transport->GetPageCountForTesting()));
	}
};

#endif
