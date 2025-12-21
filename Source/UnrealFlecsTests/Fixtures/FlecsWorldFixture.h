// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

#include "CoreMinimal.h"

#include "EngineUtils.h"
#include "UObject/Object.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

#include "Worlds/Settings/FlecsWorldInfoSettings.h"
#include "Worlds/FlecsWorldSubsystem.h"
#include "Worlds/FlecsWorld.h"
#include "Pipelines/FlecsDefaultGameLoop.h"

/*namespace Unreal::Flecs::Testing::impl
{
	static const FString DefaultTags = TEXT("");
} // namespace Unreal::Flecs::Testing::impl*/

class UNREALFLECSTESTS_API FFlecsTestFixture
{
	static constexpr double DefaultTickStepInSeconds = 1.0 / 60.0;
	
public:
	TUniquePtr<FTestWorldWrapper> TestWorldWrapper;

	TSharedPtr<FScopedTestEnvironment> TestEnvironment;

	UGameInstance* StandaloneGameInstance = nullptr;
	APlayerController* StandalonePlayerController = nullptr;
	
	TWeakObjectPtr<UWorld> TestWorld;
	
	UFlecsWorldSubsystem* WorldSubsystem = nullptr;
	UFlecsWorld* FlecsWorld = nullptr;

	// @TODO: add test support for multiple game loops
	void SetUp(TArray<TScriptInterface<IFlecsGameLoopInterface>> InGameLoopInterfaces = {}, TArray<FFlecsTickFunctionSettingsInfo> InTickFunctions = {},
	           const TArray<UObject*>& InModules = {})
	{
		TestEnvironment = FScopedTestEnvironment::Get();
		TestEnvironment->SetConsoleVariableValue("r.RayTracing.Enable", "0");
		
		TestWorldWrapper = MakeUnique<FTestWorldWrapper>();
		TestWorldWrapper->CreateTestWorld(EWorldType::Game);
		
		TestWorld = TestWorldWrapper->GetTestWorld();
		check(TestWorld.IsValid());
		
		StandaloneGameInstance = TestWorldWrapper->GetTestWorld()->GetGameInstance();
		check(IsValid(StandaloneGameInstance));

		WorldSubsystem = TestWorld->GetSubsystem<UFlecsWorldSubsystem>();
		check(IsValid(WorldSubsystem));

		/*StandalonePlayerController = NewObject<APlayerController>(TestWorld.Get());
		check(IsValid(StandalonePlayerController));*/

		// Create world settings
		FFlecsWorldSettingsInfo WorldSettings;
		WorldSettings.WorldName = "TestWorld";
		WorldSettings.Modules = InModules;

		if (!InGameLoopInterfaces.IsEmpty())
		{
			for (const TScriptInterface<IFlecsGameLoopInterface>& GameLoopInterface : InGameLoopInterfaces)
			{
				WorldSettings.GameLoops.AddUnique(GameLoopInterface.GetObject());
			}
		}
		else
		{
			WorldSettings.GameLoops.AddUnique(NewObject<UFlecsDefaultGameLoop>(WorldSubsystem));
		}

		if (!InTickFunctions.IsEmpty())
		{
			WorldSettings.TickFunctions = InTickFunctions;
		}

		FlecsWorld = WorldSubsystem->CreateWorld("TestWorld", WorldSettings);

		TestWorldWrapper->BeginPlayInTestWorld();
	}

	void TickWorld(const double InDeltaSeconds = DefaultTickStepInSeconds) const
	{
		TestWorldWrapper->TickTestWorld(InDeltaSeconds);
	}

	void TearDown()
	{
		if (FlecsWorld)
		{
			FlecsWorld = nullptr;
		}

		if (WorldSubsystem)
		{
			WorldSubsystem = nullptr;
		}

		if (TestWorld.IsValid())
		{
			TestWorld = nullptr;
		}

		if (StandaloneGameInstance)
		{
			StandaloneGameInstance = nullptr;
		}

		/*if (StandalonePlayerController)
		{
			StandalonePlayerController = nullptr;
		}*/

		if (TestWorldWrapper)
		{
			TestWorldWrapper->DestroyTestWorld(true);
			TestWorldWrapper.Reset();
		}

		if (TestEnvironment)
		{
			TestEnvironment.Reset();
		}
	}

	NO_DISCARD FORCEINLINE UFlecsWorld* GetFlecsWorld() const
	{
		return FlecsWorld;
	}
	
}; // class FFlecsTestFixture

struct UNREALFLECSTESTS_API FFlecsTestFixtureRAII
{
	mutable FFlecsTestFixture Fixture;

	FFlecsTestFixtureRAII(TArray<TScriptInterface<IFlecsGameLoopInterface>> InGameLoopInterfaces = {}, const TArray<FFlecsTickFunctionSettingsInfo> InTickFunctions = {},
			   const TArray<UObject*>& InModules = {})
	{
		Fixture.SetUp(InGameLoopInterfaces, InTickFunctions, InModules);
	}

	~FFlecsTestFixtureRAII()
	{
		Fixture.TearDown();
	}

	FORCEINLINE FFlecsTestFixture* operator->() const
	{
		return &Fixture;
	}
	
}; // struct FFlecsTestFixtureRAII

#define FLECS_FIXTURE_LIFECYCLE(FixtureName) \
	BeforeEach([this]() \
	{ \
		FixtureName.SetUp(); \
	}); \
	AfterEach([this]() \
	{ \
		FixtureName.TearDown(); \
	})

#define xTEST_METHOD_WITH_TAGS(_MethodName, _TestTags) \
	void _MethodName()

#define xTEST_METHOD(_MethodName) xTEST_METHOD_WITH_TAGS(_MethodName, Unreal::Flecs::Testing::impl::DefaultTags)

#endif // #if WITH_AUTOMATION_TESTS
