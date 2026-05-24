// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Components/PIENetworkComponent.h"
#include "UnrealFlecsConfigMacros.h"

#if ENABLE_PIE_NETWORK_TEST && ENABLE_UNREAL_FLECS_TESTS

#include "GameFramework/GameModeBase.h"

#include "General/FlecsObjectRegistrationNetworkFlags.h"
#include "Pipelines/FlecsDefaultGameLoop.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"
#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldSubsystem.h"
#include "Worlds/Settings/FlecsWorldInfoSettings.h"

namespace UE::Flecs::Tests
{
	static UFlecsWorld* CreateObjectRegistrationNetworkFlagsTestWorld(const UWorld* InWorld)
	{
		const TSolidNotNull<UFlecsWorldSubsystem*> WorldSubsystem = InWorld->GetSubsystem<UFlecsWorldSubsystem>();

		FFlecsWorldSettingsInfo Settings;
		Settings.WorldName = "ObjectRegistrationNetworkFlagsTest";
		Settings.GameLoops.AddUnique(NewObject<UFlecsDefaultGameLoop>(WorldSubsystem));

		return WorldSubsystem->CreateWorld("ObjectRegistrationNetworkFlagsTest", Settings);
	}
	
} // namespace UE::Flecs::Tests

NETWORK_TEST_CLASS(FlecsObjectRegistrationNetworkFlagsDedicatedServerTests,
	"UnrealFlecs.ObjectRegistration.NetworkFlags.DedicatedServer")
{
	struct FState : public FBasePIENetworkComponentState
	{
		UFlecsWorld* FlecsWorld = nullptr;
		UObject* RegisteredObject = nullptr;
	}; // struct FState

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

	TEST_METHOD(ServerOnlyObject_RegistersOnDedicatedServer_AndIsRejectedOnClient)
	{
		Network
			.ThenServer([this](FState& ServerState)
			{
				ASSERT_THAT(AreEqual(NM_DedicatedServer, ServerState.World->GetNetMode()));
				ASSERT_THAT(AreEqual(
					EFlecsObjectRegistrationNetworkFlags::Server,
					UE::Flecs::Net::GetFlagsForWorld(ServerState.World)));

				ServerState.FlecsWorld = UE::Flecs::Tests::CreateObjectRegistrationNetworkFlagsTestWorld(ServerState.World);
				ServerState.RegisteredObject =
					ServerState.FlecsWorld->RegisterFlecsObject<UFlecsServerOnlyObjectRegistrationTestObject>();

				ASSERT_THAT(IsNotNull(ServerState.RegisteredObject));
			})
			.ThenClients([this](FState& ClientState)
			{
				ASSERT_THAT(AreEqual(NM_Client, ClientState.World->GetNetMode()));
				ASSERT_THAT(AreEqual(
					EFlecsObjectRegistrationNetworkFlags::Client,
					UE::Flecs::Net::GetFlagsForWorld(ClientState.World)));

				ClientState.FlecsWorld = UE::Flecs::Tests::CreateObjectRegistrationNetworkFlagsTestWorld(ClientState.World);
				ClientState.RegisteredObject =
					ClientState.FlecsWorld->RegisterFlecsObject<UFlecsServerOnlyObjectRegistrationTestObject>();

				ASSERT_THAT(IsNull(ClientState.RegisteredObject));
			});
	}

	TEST_METHOD(ClientOnlyObject_IsRejectedOnDedicatedServer_AndRegistersOnClient)
	{
		Network
			.ThenServer([this](FState& ServerState)
			{
				ASSERT_THAT(AreEqual(NM_DedicatedServer, ServerState.World->GetNetMode()));

				ServerState.FlecsWorld = UE::Flecs::Tests::CreateObjectRegistrationNetworkFlagsTestWorld(ServerState.World);
				ServerState.RegisteredObject =
					ServerState.FlecsWorld->RegisterFlecsObject<UFlecsClientOnlyObjectRegistrationTestObject>();

				ASSERT_THAT(IsNull(ServerState.RegisteredObject));
			})
			.ThenClients([this](FState& ClientState)
			{
				ASSERT_THAT(AreEqual(NM_Client, ClientState.World->GetNetMode()));

				ClientState.FlecsWorld = UE::Flecs::Tests::CreateObjectRegistrationNetworkFlagsTestWorld(ClientState.World);
				ClientState.RegisteredObject =
					ClientState.FlecsWorld->RegisterFlecsObject<UFlecsClientOnlyObjectRegistrationTestObject>();

				ASSERT_THAT(IsNotNull(ClientState.RegisteredObject));
			});
	}
	
}; // End of NETWORK_TEST_CLASS(FlecsObjectRegistrationNetworkFlagsDedicatedServerTests)

NETWORK_TEST_CLASS(FlecsObjectRegistrationNetworkFlagsListenServerTests,
	"UnrealFlecs.ObjectRegistration.NetworkFlags.ListenServer")
{
	struct FState : public FBasePIENetworkComponentState
	{
		UFlecsWorld* FlecsWorld = nullptr;
		UObject* RegisteredObject = nullptr;
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

	TEST_METHOD(ListenServerWorld_UsesServerAndClientFlags)
	{
		Network.ThenServer([this](FState& ServerState)
			{
				ASSERT_THAT(AreEqual(NM_ListenServer, ServerState.World->GetNetMode()));

				const EFlecsObjectRegistrationNetworkFlags WorldFlags =
					UE::Flecs::Net::GetFlagsForWorld(ServerState.World);

				ASSERT_THAT(IsTrue(EnumHasAllFlags(WorldFlags, EFlecsObjectRegistrationNetworkFlags::Server)));
				ASSERT_THAT(IsTrue(EnumHasAllFlags(WorldFlags, EFlecsObjectRegistrationNetworkFlags::Client)));

				ServerState.FlecsWorld = UE::Flecs::Tests::CreateObjectRegistrationNetworkFlagsTestWorld(ServerState.World);

				ServerState.RegisteredObject =
					ServerState.FlecsWorld->RegisterFlecsObject<UFlecsServerOnlyObjectRegistrationTestObject>();
				ASSERT_THAT(IsNotNull(ServerState.RegisteredObject));

				ServerState.RegisteredObject =
					ServerState.FlecsWorld->RegisterFlecsObject<UFlecsClientOnlyObjectRegistrationTestObject>();
				ASSERT_THAT(IsNotNull(ServerState.RegisteredObject));
			});
	}
	
}; // End of NETWORK_TEST_CLASS(FlecsObjectRegistrationNetworkFlagsListenServerTests)

#endif // ENABLE_PIE_NETWORK_TEST && ENABLE_UNREAL_FLECS_TESTS
