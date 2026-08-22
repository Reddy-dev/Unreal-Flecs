// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "UnrealFlecsConfigMacros.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS && ENABLE_PIE_NETWORK_TEST

#include "Components/PIENetworkComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Pipelines/FlecsDefaultGameLoop.h"
#include "Systems/FlecsPhasesType.h"
#include "Systems/FlecsSystemHandle.h"
#include "UObject/UObjectGlobals.h"
#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldSubsystem.h"
#include "Worlds/Settings/FlecsWorldInfoSettings.h"

namespace UE::Flecs::Tests
{
	static UFlecsWorld* CreateSystemRunPIEWorld(const UWorld* InWorld)
	{
		const TSolidNotNull<UFlecsWorldSubsystem*> WorldSubsystem =
			InWorld->GetSubsystem<UFlecsWorldSubsystem>();

		FFlecsWorldSettingsInfo Settings;
		Settings.WorldName = TEXT("FlecsSystemRunPIE");
		Settings.GameLoops.AddUnique(NewObject<UFlecsDefaultGameLoop>(WorldSubsystem));

		return WorldSubsystem->CreateWorld(TEXT("FlecsSystemRunPIE"), Settings);
	}
} // namespace UE::Flecs::Tests

NETWORK_TEST_CLASS(FlecsDisabledSystemPIETests,
	"UnrealFlecs.Pipelines.SystemRun.PIE")
{
	struct FState : FBasePIENetworkComponentState
	{
		UFlecsWorld* FlecsWorld = nullptr;
		FFlecsSystemHandle DisabledSystem;
		int32 RunCount = 0;
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

	TEST_METHOD(DisabledSystem_IsNotRunByPIEPipelineTick)
	{
		Network
			.ThenServer([](FState& State)
			{
				State.FlecsWorld = UE::Flecs::Tests::CreateSystemRunPIEWorld(State.World);
				State.DisabledSystem = State.FlecsWorld->CreateSystem<>(TEXT("PIEDisabledSystem"))
					.Phase(EFlecsPhaseType::OnUpdate)
					.run([&State](flecs::iter& InIterator)
					{
						while (InIterator.next())
						{
							++State.RunCount;
						}
					});
				
				State.DisabledSystem.Disable();
			})
			.ThenServer(TEXT("Disabled system remains outside the PIE pipeline after a world tick"), [this](FState& State)
			{
				ASSERT_THAT(IsTrue(State.DisabledSystem.IsValid()));
				ASSERT_THAT(IsTrue(State.DisabledSystem.Has(flecs::Disabled)));
				ASSERT_THAT(AreEqual(0, State.RunCount));
				
				State.DisabledSystem.Run();
				ASSERT_THAT(IsTrue(State.DisabledSystem.IsValid()));
				ASSERT_THAT(IsTrue(State.DisabledSystem.Has(flecs::Disabled)));
				ASSERT_THAT(AreEqual(1, State.RunCount));
			});
	}
}; // FlecsDisabledSystemPIETests

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS && ENABLE_PIE_NETWORK_TEST
