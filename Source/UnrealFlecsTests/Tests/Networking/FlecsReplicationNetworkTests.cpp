// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "UnrealFlecsConfigMacros.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS && ENABLE_PIE_NETWORK_TEST

#include "Components/PIENetworkComponent.h"
#include "Engine/NetDriver.h"
#include "GameFramework/GameModeBase.h"

#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"
#include "Networking/FlecsReplicatedEntityComponent.h"
#include "Networking/FlecsReplicationBridgeBase.h"
#include "Pipelines/FlecsDefaultGameLoop.h"
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
		InWorld->CreateQueryBuilder<const FFlecsReplicationTestValue>()
			.With<FFlecsNetworkId>()
			.Build()
			.each([&Result](flecs::iter& InIter, size_t InIndex, const FFlecsReplicationTestValue&)
			{
				if (!Result.IsValid())
				{
					Result = InIter.entity(InIndex);
				}
			});

		return Result;
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
				const FFlecsEntityHandle Entity =
					UE::Flecs::Tests::FindReplicatedValueEntity(State.FlecsWorld);
				return Entity.IsValid() && Entity.Get<FFlecsReplicationTestValue>().Value == 73;
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
}; // FlecsReplicationRealBridgeNetworkTests

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS && ENABLE_PIE_NETWORK_TEST
