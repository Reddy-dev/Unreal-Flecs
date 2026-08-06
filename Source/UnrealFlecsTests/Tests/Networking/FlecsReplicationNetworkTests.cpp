// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "UnrealFlecsConfigMacros.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS && ENABLE_PIE_NETWORK_TEST

#include "Components/PIENetworkComponent.h"
#include "Engine/NetDriver.h"
#include "GameFramework/GameModeBase.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"

#include "Networking/Bridge/FlecsIrisReplicationBridge.h"
#include "Networking/FlecsReplicatedEntityComponent.h"
#include "Networking/Bridge/FlecsReplicationBridgeBase.h"
#include "Networking/Shards/FlecsNetEntityProxy.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"
#include "Pipelines/FlecsDefaultGameLoop.h"
#include "Queries/FlecsQuery.h"
#include "UnrealFlecsTests/Fixtures/FlecsTestReplicationBridge.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"
#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldSubsystem.h"
#include "Worlds/Settings/FlecsWorldInfoSettings.h"

namespace UE::Flecs::Tests
{
	static UFlecsWorld* CreateNetworkTestWorld(const UWorld* InWorld)
	{
		const TSolidNotNull<UFlecsWorldSubsystem*> WorldSubsystem =
			InWorld->GetSubsystem<UFlecsWorldSubsystem>();

		FFlecsWorldSettingsInfo Settings;
		Settings.WorldName = TEXT("FlecsReplicationPIE");
		Settings.GameLoops.AddUnique(NewObject<UFlecsDefaultGameLoop>(WorldSubsystem));

		UFlecsWorld* World = WorldSubsystem->CreateWorld(TEXT("FlecsReplicationPIE"), Settings);

		const FFlecsComponentHandle Component = World->RegisterComponentType<FFlecsReplicationTestValue>();
		const FFlecsComponentPropertiesDefinition Properties =
			FFlecsComponentPropertiesDefinition::Make<FFlecsReplicationTestValue>();
		Properties.PropertiesFunction(World, Component, Properties);

		return World;
	}

	static FFlecsEntityHandle FindReplicatedValueEntity(UFlecsWorld* InWorld)
	{
		FFlecsEntityHandle Result;
		const TTypedFlecsQuery<FFlecsReplicationTestValue> Query =
			InWorld->CreateQueryBuilder<const FFlecsReplicationTestValue>()
			.With<FFlecsNetworkId>()
			.Build();

		Query.each([&Result](flecs::iter& InIter, size_t InIndex, const FFlecsReplicationTestValue)
		{
			if (!Result.IsValid())
			{
				Result = InIter.entity(InIndex);
			}
		});

		return Result;
	}

	static UFlecsNetEntityProxy* FindReplicatedEntityProxy(const UWorld* InWorld)
	{
		for (TObjectIterator<UFlecsNetEntityProxy> It; It; ++It)
		{
			UFlecsNetEntityProxy* Proxy = *It;
			if (Proxy->GetWorld() == InWorld && Proxy->NetworkId.IsValid() && !Proxy->IsEmpty())
			{
				return Proxy;
			}
		}

		return nullptr;
	}
} // namespace UE::Flecs::Tests

NETWORK_TEST_CLASS(FlecsReplicationRealBridgeNetworkTests,
	"UnrealFlecs.Networking.Replication.RealBridge.PIE")
{
	struct FState : FBasePIENetworkComponentState
	{
		UFlecsWorld* FlecsWorld = nullptr;
		FFlecsEntityHandle AuthorityEntity;
	};

	FPIENetworkComponent<FState> Network{ TestRunner, TestCommandBuilder, bInitializing };

	BEFORE_EACH()
	{
		FNetworkComponentBuilder<FState>()
			.WithClients(1)
			.AsDedicatedServer()
			.WithGameInstanceClass(UGameInstance::StaticClass())
			.WithGameMode(AGameModeBase::StaticClass())
			.Build(Network);
	}

	TEST_METHOD(ConfiguredRealBridge_IsReplicatedToClients)
	{
		Network
			.ThenServer([](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateNetworkTestWorld(State.World);
			})
			.ThenClients([](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateNetworkTestWorld(State.World);
			})
			.UntilClients(TEXT("Iris creates and binds the replicated Flecs bridge"), [](FState& State)
			{
				const UFlecsNetworkWorldSubsystem* NetworkSubsystem =
					State.World->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();

				return NetworkSubsystem->HasReplicationBridge()
					&& NetworkSubsystem->GetReplicationBridge()->IsA<UFlecsIrisReplicationBridge>();
			});
	}

	TEST_METHOD(ConfiguredRealBridge_ReplicatesInitialEntitySnapshot)
	{
		Network
			.ThenServer([this](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateNetworkTestWorld(State.World);
				UFlecsReplicationBridgeBase* Bridge =
					State.World->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>()->GetReplicationBridge();
				ASSERT_THAT(IsNotNull(Bridge));
				ASSERT_THAT(IsFalse(Bridge->IsA<UFlecsTestReplicationBridge>()));
			})
			.ThenClients([](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateNetworkTestWorld(State.World);
			})
			.ThenServer([](FState& State)
			{
				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 73 })
					.Add<FFlecsReplicatedEntityComponent>();
			})
			.UntilClients(TEXT("Configured bridge replicates the initial Flecs entity"), [](FState& State)
			{
				const FFlecsEntityHandle Entity = UE::Flecs::Tests::FindReplicatedValueEntity(State.FlecsWorld);
				return Entity.IsValid() && Entity.Get<FFlecsReplicationTestValue>().Value == 73;
			});
	}

	TEST_METHOD(ConfiguredRealBridge_BindsReceivedProxyToClientWorld)
	{
		Network
			.ThenServer([](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateNetworkTestWorld(State.World);
				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 47 })
					.Add<FFlecsReplicatedEntityComponent>();
			})
			.ThenClients([](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateNetworkTestWorld(State.World);
			})
			.UntilClients(TEXT("Received Flecs proxy is assigned to the client world before applying its snapshot"),
				[](FState& State)
			{
					const FFlecsEntityHandle Entity = UE::Flecs::Tests::FindReplicatedValueEntity(State.FlecsWorld);
					UFlecsNetEntityProxy* Proxy = UE::Flecs::Tests::FindReplicatedEntityProxy(State.World);
					return Entity.IsValid()
						&& Entity.Get<FFlecsReplicationTestValue>().Value == 47
						&& Proxy
						&& Proxy->GetWorld() == State.World;
				})
			.ThenClients([this](FState& State)
			{
				UFlecsNetEntityProxy* Proxy = UE::Flecs::Tests::FindReplicatedEntityProxy(State.World);
				ASSERT_THAT(IsNotNull(Proxy));
				if (!Proxy)
				{
					return;
				}

				ASSERT_THAT(IsTrue(Proxy->GetOuter() == GetTransientPackage()));
				ASSERT_THAT(IsTrue(Proxy->GetWorld() == State.World));
			});
	}

	TEST_METHOD(ConfiguredRealBridge_DefersInitialSnapshotUntilClientFlecsWorldExists)
	{
		Network
			.ThenServer([](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateNetworkTestWorld(State.World);
				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 59 })
					.Add<FFlecsReplicatedEntityComponent>();
			})
			.UntilClients(TEXT("Client queues the initial Flecs snapshot until its local Flecs world exists"),
				[](FState& State)
			{
					const UFlecsNetworkWorldSubsystem* NetworkSubsystem =
						State.World->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
					return !State.FlecsWorld
						&& NetworkSubsystem->HasReplicationBridge()
						&& NetworkSubsystem->GetQueuedReplicationUpdateCount() > 0
						&& UE::Flecs::Tests::FindReplicatedEntityProxy(State.World) != nullptr;
				})
			.ThenClients([](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateNetworkTestWorld(State.World);
			})
			.UntilClients(TEXT("Queued initial Flecs snapshot is applied after the client Flecs world is created"),
				[](FState& State)
			{
					const FFlecsEntityHandle Entity =
						UE::Flecs::Tests::FindReplicatedValueEntity(State.FlecsWorld);
					return Entity.IsValid() && Entity.Get<FFlecsReplicationTestValue>().Value == 59;
				});
	}

	TEST_METHOD(ConfiguredRealBridge_ReplicatesRuntimeValueChange)
	{
		Network
			.ThenServer([](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateNetworkTestWorld(State.World);
			})
			.ThenClients([](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateNetworkTestWorld(State.World);
			})
			.ThenServer([](FState& State)
			{
				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 11 })
					.Add<FFlecsReplicatedEntityComponent>();
			})
			.UntilClients([](FState& State)
			{
				const FFlecsEntityHandle Entity =
					UE::Flecs::Tests::FindReplicatedValueEntity(State.FlecsWorld);
				return Entity.IsValid() && Entity.Get<FFlecsReplicationTestValue>().Value == 11;
			})
			.ThenServer([](FState& State)
			{
				State.AuthorityEntity.Set<FFlecsReplicationTestValue>({ 91 });
			})
			.UntilClients(TEXT("Configured bridge replicates a runtime value change"), [](FState& State)
			{
				const FFlecsEntityHandle Entity =
					UE::Flecs::Tests::FindReplicatedValueEntity(State.FlecsWorld);
				return Entity.IsValid() && Entity.Get<FFlecsReplicationTestValue>().Value == 91;
			});
	}

	TEST_METHOD(ConfiguredRealBridge_RemovesEntityWhenReplicationMarkerIsRemoved)
	{
		Network
			.ThenServer([](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateNetworkTestWorld(State.World);
			})
			.ThenClients([](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateNetworkTestWorld(State.World);
			})
			.ThenServer([](FState& State)
			{
				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 37 })
					.Add<FFlecsReplicatedEntityComponent>();
			})
			.UntilClients([](FState& State)
			{
				return UE::Flecs::Tests::FindReplicatedValueEntity(State.FlecsWorld).IsValid();
			})
			.ThenServer([](FState& State)
			{
				State.AuthorityEntity.Remove<FFlecsReplicatedEntityComponent>();
			})
			.UntilClients(TEXT("Removing the replication marker detaches the proxy and destroys the remote entity"),
				[](FState& State)
			{
				return !UE::Flecs::Tests::FindReplicatedValueEntity(State.FlecsWorld).IsValid();
			});
	}
}; // FlecsReplicationRealBridgeNetworkTests

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS && ENABLE_PIE_NETWORK_TEST
