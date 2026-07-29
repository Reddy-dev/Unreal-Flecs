// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "UnrealFlecsConfigMacros.h"
#include "UnrealFlecsTests/Fixtures/FlecsReplicationFixture.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Networking/FlecsNetDirtyTag.h"
#include "Networking/FlecsNetworkingModuleSettings.h"
#include "Networking/FlecsReplicatedEntityComponent.h"
#include "Networking/Layout/FlecsLayoutReplicatorFastArray.h"
#include "Networking/Layout/FlecsReplicationLayoutRegistry.h"
#include "Networking/Router/FlecsDefaultReplicationRouter.h"
#include "Networking/Shards/FlecsNetEntityPage.h"
#include "Networking/Shards/FlecsNetEntityProxy.h"
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

	TEST_METHOD(EntityPageAndEntityProxy_AreShardStorageTypes)
	{
		ASSERT_THAT(IsTrue(
			UFlecsNetEntityPage::StaticClass()->IsChildOf(UFlecsNetShardBase::StaticClass())));
		ASSERT_THAT(IsTrue(
			UFlecsNetEntityProxy::StaticClass()->IsChildOf(UFlecsNetShardBase::StaticClass())));
	}

	TEST_METHOD(ConfiguredRouter_IsCreatedByTheNetworkingSubsystem)
	{
		UFlecsReplicationRouterBase* Router = NetworkSubsystem()->GetReplicationRouter();

		ASSERT_THAT(IsTrue(
			Router->GetClass() == GetDefault<UFlecsNetworkingModuleSettings>()->ReplicationRouterClass.Get()));
	}

	TEST_METHOD(DefaultRouter_RoutesEveryEntityToDefault)
	{
		const UFlecsDefaultReplicationRouter* Router =
			NewObject<UFlecsDefaultReplicationRouter>(NetworkSubsystem());
		const FFlecsEntityHandle Entity = World()->CreateEntity();

		ASSERT_THAT(IsTrue(Router->RouteEntity(Entity) == FFlecsNetRouteId::Default()));
	}

	TEST_METHOD(LayoutFastArray_AddsIdempotentlyAndExplicitlyRemoves)
	{
		FFlecsReplicationLayoutDefinition Layout;
		Layout.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());

		FFlecsReplicatorFastArray Layouts;

		ASSERT_THAT(IsTrue(Layouts.AddLayout(Layout)));
		ASSERT_THAT(IsFalse(Layouts.AddLayout(Layout)));
		ASSERT_THAT(IsTrue(Layouts.Items.Num() == 1));
		ASSERT_THAT(IsTrue(Layouts.Items[0].ReplicationID != INDEX_NONE));
		ASSERT_THAT(IsTrue(Layouts.FindLayout(Layout.LayoutId) != nullptr));

		ASSERT_THAT(IsTrue(Layouts.RemoveLayout(Layout.LayoutId)));
		ASSERT_THAT(IsFalse(Layouts.RemoveLayout(Layout.LayoutId)));
		ASSERT_THAT(IsTrue(Layouts.Items.IsEmpty()));
	}

	TEST_METHOD(LayoutRegistry_RetainsLayoutUntilFinalTableIsDeleted)
	{
		const FFlecsEntityHandle FirstEntity = World()->CreateEntity()
			.Add<FFlecsReplicatedEntityComponent>()
			.Add<FFlecsReplicationTestTag>();
		const FFlecsEntityHandle SecondEntity = World()->CreateEntity()
			.Add<FFlecsReplicatedEntityComponent>()
			.Add<FFlecsReplicationTestTag>()
			.Add<FFlecsNetDirtyTag>();

		bool bCreatedFirstLayout = false;
		bool bCreatedSecondLayout = false;
		FFlecsReplicationLayoutRegistry& Registry = NetworkSubsystem()->GetLayoutRegistry();

		const TValueOrError<const FFlecsReplicationLayoutDefinition*, FString> FirstResult =
			Registry.BuildForEntity(World(), FirstEntity, bCreatedFirstLayout);
		const TValueOrError<const FFlecsReplicationLayoutDefinition*, FString> SecondResult =
			Registry.BuildForEntity(World(), SecondEntity, bCreatedSecondLayout);

		ASSERT_THAT(IsFalse(FirstResult.HasError()));
		ASSERT_THAT(IsFalse(SecondResult.HasError()));
		ASSERT_THAT(IsTrue(bCreatedFirstLayout));
		ASSERT_THAT(IsFalse(bCreatedSecondLayout));

		const FFlecsReplicationLayoutId LayoutId = FirstResult.GetValue()->LayoutId;
		ASSERT_THAT(IsTrue(LayoutId == SecondResult.GetValue()->LayoutId));

		const flecs::table_t* FirstTable = FirstEntity.GetEntity().table().get_table();
		const flecs::table_t* SecondTable = SecondEntity.GetEntity().table().get_table();

		ASSERT_THAT(IsFalse(Registry.RemoveLocalTable(FirstTable).IsSet()));
		ASSERT_THAT(IsNotNull(Registry.Find(LayoutId)));

		const TOptional<FFlecsReplicationLayoutId> RemovedLayout = Registry.RemoveLocalTable(SecondTable);
		ASSERT_THAT(IsTrue(RemovedLayout.IsSet()));
		ASSERT_THAT(IsTrue(RemovedLayout.GetValue() == LayoutId));
		ASSERT_THAT(IsNull(Registry.Find(LayoutId)));
	}

	
}; // FlecsReplicationBridgeTests

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS
