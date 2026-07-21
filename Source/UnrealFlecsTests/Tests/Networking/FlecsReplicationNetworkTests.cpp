// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Components/PIENetworkComponent.h"
#include "UnrealFlecsConfigMacros.h"

#if ENABLE_PIE_NETWORK_TEST && ENABLE_UNREAL_FLECS_TESTS

#include "GameFramework/GameModeBase.h"

#include "Networking/FlecsNetworkId.h"
#include "Networking/FlecsReplicatedEntityComponent.h"
#include "Pipelines/FlecsDefaultGameLoop.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"
#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldSubsystem.h"
#include "Worlds/Settings/FlecsWorldInfoSettings.h"

namespace UE::Flecs::Tests
{
	template<typename T>
	void RegisterPIEComponent(UFlecsWorld* World)
	{
		const FFlecsComponentHandle Component = World->RegisterComponentType<T>();
		const FFlecsComponentPropertiesDefinition Properties = FFlecsComponentPropertiesDefinition::Make<T>();
		Properties.PropertiesFunction(World, Component, Properties);
	}

	static UFlecsWorld* CreateReplicationPIEWorld(const UWorld* InWorld)
	{
		const TSolidNotNull<UFlecsWorldSubsystem*> WorldSubsystem = InWorld->GetSubsystem<UFlecsWorldSubsystem>();
		
		FFlecsWorldSettingsInfo Settings;
		Settings.WorldName = TEXT("FlecsReplicationPIE");
		Settings.GameLoops.AddUnique(NewObject<UFlecsDefaultGameLoop>(WorldSubsystem));
		
		UFlecsWorld* World = WorldSubsystem->CreateWorld(TEXT("FlecsReplicationPIE"), Settings);
		
		RegisterPIEComponent<FFlecsReplicationTestRequiredTag>(World);
		RegisterPIEComponent<FFlecsReplicationTestValue>(World);
		RegisterPIEComponent<FFlecsReplicationTestNativeValue>(World);
		RegisterPIEComponent<FFlecsReplicationTestTag>(World);
		RegisterPIEComponent<FFlecsReplicationTestRelationship>(World);
		RegisterPIEComponent<FFlecsReplicationTestWithValue>(World);
		RegisterPIEComponent<FFlecsReplicationTestLocalOnly>(World);
		
		return World;
	}

	static FFlecsEntityHandle FindReplicatedValueEntity(UFlecsWorld* World)
	{
		FFlecsEntityHandle Result;
		
		TTypedFlecsQuery<FFlecsReplicationTestValue> Query =
			World->CreateQueryBuilder<const FFlecsReplicationTestValue>()
			.With<FFlecsNetworkId>()
			.With<FFlecsReplicatedEntityComponent>()
			.Build();
		
		Query.each([&Result](flecs::iter& Iter, size_t Index, const FFlecsReplicationTestValue&)
		{
			if (!Result.IsValid())
			{
				Result = Iter.entity(Index);
			}
		});
		
		return Result;
	}

	static int32 CountReplicatedEntities(UFlecsWorld* World)
	{
		return World->CreateQueryBuilder<>()
			.With<FFlecsNetworkId>()
			.With<FFlecsReplicatedEntityComponent>()
			.Build()
			.count();
	}
}

NETWORK_TEST_CLASS(FlecsReplicationDedicatedServerTests,
	"UnrealFlecs.Networking.Replication.PIE.DedicatedServer")
{
	struct FState : FBasePIENetworkComponentState
	{
		UFlecsWorld* FlecsWorld = nullptr;
		FFlecsEntityHandle Entity;
		FFlecsEntityHandle Target;
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

	TEST_METHOD(InitialSnapshot_RuntimeCompositionAndValues_StopReplication_PreserveClientLocal)
	{
		Network
			.ThenServer([](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateReplicationPIEWorld(State.World);
			})
			.ThenClients([](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateReplicationPIEWorld(State.World);
			})
			.ThenServer([](FState& State)
			{
				State.Entity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 10 })
					.Set<FFlecsReplicationTestNativeValue>({ 20 })
					.Add<FFlecsReplicationTestTag>()
					.Add<FFlecsReplicatedEntityComponent>();
			})
			.UntilClients(TEXT("Client receives initial replicated composition and values"), [](FState& State)
			{
				const FFlecsEntityHandle Entity = UE::Flecs::Tests::FindReplicatedValueEntity(State.FlecsWorld);
				
				return Entity.IsValid()
					&& Entity.Get<FFlecsReplicationTestValue>().Value == 10
					&& Entity.Get<FFlecsReplicationTestNativeValue>().Value == 20
					&& Entity.Has<FFlecsReplicationTestTag>();
			})
			.ThenClients([](FState& State)
			{
				UE::Flecs::Tests::FindReplicatedValueEntity(State.FlecsWorld)
					.Set<FFlecsReplicationTestLocalOnly>({ 77 });
			})
			.ThenServer([](FState& State)
			{
				State.Entity.Set<FFlecsReplicationTestValue>({ 100 });
				State.Entity.Set<FFlecsReplicationTestNativeValue>({ 200 });
				State.Entity.Remove<FFlecsReplicationTestTag>();
				State.Entity.Set<FFlecsReplicationTestWithValue>({ 300 });
				State.Entity.Add<FFlecsReplicationTestTag>().Remove<FFlecsReplicationTestTag>();
			})
			.UntilClients(TEXT("Client receives runtime composition and value changes"), [](FState& State)
			{
				const FFlecsEntityHandle Entity = UE::Flecs::Tests::FindReplicatedValueEntity(State.FlecsWorld);
				return Entity.IsValid()
					&& Entity.Get<FFlecsReplicationTestValue>().Value == 100
					&& Entity.Get<FFlecsReplicationTestNativeValue>().Value == 200
					&& Entity.Get<FFlecsReplicationTestWithValue>().Value == 300
					&& Entity.Has<FFlecsReplicationTestRequiredTag>()
					&& !Entity.Has<FFlecsReplicationTestTag>()
					&& Entity.Get<FFlecsReplicationTestLocalOnly>().Value == 77;
			})
			.ThenServer([](FState& State)
			{
				State.Entity.Remove<FFlecsReplicatedEntityComponent>();
			})
			.UntilClients(TEXT("Client removes entity after replication opt-out"), [](FState& State)
			{
				return UE::Flecs::Tests::CountReplicatedEntities(State.FlecsWorld) == 0;
			});
	}

	TEST_METHOD(StaticAndEntityTargetPairs_ReplicateAndResolve)
	{
		Network
			.ThenServer([](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateReplicationPIEWorld(State.World);
			})
			.ThenClients([](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateReplicationPIEWorld(State.World);
			})
			.ThenServer([](FState& State)
			{
				State.Target = State.FlecsWorld->CreateEntity().Add<FFlecsReplicatedEntityComponent>();
				State.Entity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 5 })
					.AddPair<FFlecsReplicationTestRelationship, FFlecsReplicationTestTag>()
					.AddPair<FFlecsReplicationTestRelationship>(State.Target)
					.Add<FFlecsReplicatedEntityComponent>();
			})
			.UntilClients([](FState& State)
			{
				const FFlecsEntityHandle Source = UE::Flecs::Tests::FindReplicatedValueEntity(State.FlecsWorld);
				
				if (!Source.IsValid() || UE::Flecs::Tests::CountReplicatedEntities(State.FlecsWorld) != 2)
				{
					return false;
				}
				
				bool bFoundEntityTarget = false;
				const FFlecsId Relationship = State.FlecsWorld->GetIdIfRegistered<FFlecsReplicationTestRelationship>();
				for (int32 Index = 0; ; ++Index)
				{
					const FFlecsEntityHandle Target = Source.GetPairTarget<FFlecsEntityHandle>(Relationship, Index);
					
					if (!Target.IsValid())
					{
						break;
					}
					
					bFoundEntityTarget |= Target.Has<FFlecsNetworkId>();
				}
				
				return bFoundEntityTarget && Source.HasPair<FFlecsReplicationTestRelationship, FFlecsReplicationTestTag>();
			});
	}

	TEST_METHOD(AuthoritativeDestruction_RemovesRemoteEntity)
	{
		Network
			.ThenServer([](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateReplicationPIEWorld(State.World);
			})
			.ThenClients([](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateReplicationPIEWorld(State.World);
			})
			.ThenServer([](FState& State)
			{
				State.Entity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 9 })
					.Add<FFlecsReplicatedEntityComponent>();
			})
			.UntilClients(TEXT("Client receives entity before authoritative destruction"),
				[](FState& State)
				{
					return UE::Flecs::Tests::CountReplicatedEntities(State.FlecsWorld) == 1;
				})
			.ThenServer([](FState& State)
			{
				State.Entity.Destroy();
			})
			.UntilClients(TEXT("Client removes authoritatively destroyed entity"),
				[](FState& State)
				{
					return UE::Flecs::Tests::CountReplicatedEntities(State.FlecsWorld) == 0;
				});
	}

	TEST_METHOD(LateJoiningClient_ReceivesCurrentCompositionAndValues)
	{
		Network
			.ThenServer([](FState& State) { State.FlecsWorld = UE::Flecs::Tests::CreateReplicationPIEWorld(State.World); })
			.ThenClients([](FState& State) { State.FlecsWorld = UE::Flecs::Tests::CreateReplicationPIEWorld(State.World); })
			.ThenServer([](FState& State)
			{
				State.Entity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 55 })
					.Add<FFlecsReplicationTestTag>()
					.Add<FFlecsReplicatedEntityComponent>();
			})
			.UntilClient(0, [](FState& State)
			{
				return UE::Flecs::Tests::FindReplicatedValueEntity(State.FlecsWorld).IsValid();
			})
			.ThenClientJoins()
			.ThenClient(1, [](FState& State) { State.FlecsWorld = UE::Flecs::Tests::CreateReplicationPIEWorld(State.World); })
			.UntilClient(1, [](FState& State)
			{
				const FFlecsEntityHandle Entity = UE::Flecs::Tests::FindReplicatedValueEntity(State.FlecsWorld);
				return Entity.IsValid() && Entity.Get<FFlecsReplicationTestValue>().Value == 55
					&& Entity.Has<FFlecsReplicationTestTag>();
			});
	}
};

NETWORK_TEST_CLASS(FlecsReplicationListenServerTests,
	"UnrealFlecs.Networking.Replication.PIE.ListenServer")
{
	struct FState : FBasePIENetworkComponentState
	{
		UFlecsWorld* FlecsWorld = nullptr;
		FFlecsEntityHandle Entity;
	};
	
	FPIENetworkComponent<FState> Network{ TestRunner, TestCommandBuilder, bInitializing };

	BEFORE_EACH()
	{
		FNetworkComponentBuilder<FState>()
			.WithClients(1)
			.AsListenServer()
			.WithGameInstanceClass(UGameInstance::StaticClass())
			.WithGameMode(AGameModeBase::StaticClass())
			.Build(Network);
	}

	TEST_METHOD(InitialSnapshot_ReplicatesFromListenServer)
	{
		Network
			.ThenServer([](FState& State) { State.FlecsWorld = UE::Flecs::Tests::CreateReplicationPIEWorld(State.World); })
			.ThenClients([](FState& State) { State.FlecsWorld = UE::Flecs::Tests::CreateReplicationPIEWorld(State.World); })
			.ThenServer([](FState& State)
			{
				State.Entity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 88 })
					.Add<FFlecsReplicatedEntityComponent>();
			})
			.UntilClients([](FState& State)
			{
				const FFlecsEntityHandle Entity = UE::Flecs::Tests::FindReplicatedValueEntity(State.FlecsWorld);
				return Entity.IsValid() && Entity.Get<FFlecsReplicationTestValue>().Value == 88;
			});
	}
};

NETWORK_TEST_CLASS(FlecsReplicationIrisDisabledTests,
	"UnrealFlecs.Networking.Replication.PIE.IrisDisabled")
{
	struct FState : FBasePIENetworkComponentState
	{
		UFlecsWorld* FlecsWorld = nullptr;
	};
	FPIENetworkComponent<FState> Network{ TestRunner, TestCommandBuilder, bInitializing };
	inline static TSharedPtr<FScopedTestEnvironment> TestEnvironment;

	BEFORE_ALL()
	{
		TestEnvironment = FScopedTestEnvironment::Get();
		TestEnvironment->SetConsoleVariableValue(TEXT("net.Iris.UseIrisReplication"), TEXT("0"));
	}

	BEFORE_EACH()
	{
		FNetworkComponentBuilder<FState>()
			.WithClients(1)
			.AsDedicatedServer()
			.WithGameInstanceClass(UGameInstance::StaticClass())
			.WithGameMode(AGameModeBase::StaticClass())
			.Build(Network);
	}

	AFTER_ALL()
	{
		TestEnvironment = nullptr;
	}

	TEST_METHOD(ServerWorld_RemainsUsableWithoutIrisTransport)
	{
		Network.ThenServer([this](FState& State)
		{
			State.FlecsWorld = UE::Flecs::Tests::CreateReplicationPIEWorld(State.World);
			const FFlecsEntityHandle Entity = State.FlecsWorld->CreateEntity()
				.Set<FFlecsReplicationTestValue>({ 4 })
				.Add<FFlecsReplicatedEntityComponent>();
			ASSERT_THAT(IsTrue(Entity.IsValid()));
			ASSERT_THAT(AreEqual(4, Entity.Get<FFlecsReplicationTestValue>().Value));
		});
	}
};

#endif
