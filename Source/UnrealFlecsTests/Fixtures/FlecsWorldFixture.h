// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CQTest.h"
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

class UNREALFLECSTESTS_API FFlecsTestFixture
{
	static constexpr double DefaultTickStepInSeconds = 1.0 / 60.0;

public:
	~FFlecsTestFixture()
	{
		TearDown();
	}

	TUniquePtr<FTestWorldWrapper> TestWorldWrapper;

	TSharedPtr<FScopedTestEnvironment> TestEnvironment;

	UGameInstance* StandaloneGameInstance = nullptr;
	APlayerController* StandalonePlayerController = nullptr;

	TWeakObjectPtr<UWorld> TestWorld;

	UFlecsWorldSubsystem* WorldSubsystem = nullptr;
	UFlecsWorld* FlecsWorld = nullptr;

	void SetUp(TArray<TScriptInterface<IFlecsGameLoopInterface>> InGameLoopInterfaces = {}, TArray<FFlecsTickFunctionSettingsInfo> InTickFunctions = {},
		EWorldType::Type InWorldType = EWorldType::GameRPC, const bool bInUseDefaultGameLoop = true)
	{
		TearDown();
		TestEnvironment = FScopedTestEnvironment::Get();

		TestWorldWrapper = MakeUnique<FTestWorldWrapper>();
		TestWorldWrapper->CreateTestWorld(InWorldType);

		TestWorld = TestWorldWrapper->GetTestWorld();
		check(TestWorld.IsValid());

		WorldSubsystem = TestWorld->GetSubsystem<UFlecsWorldSubsystem>();
		check(IsValid(WorldSubsystem));

		WorldSubsystem->AddToRoot();

		FFlecsWorldSettingsInfo WorldSettings;
		WorldSettings.WorldName = "TestWorld";

		if (!InGameLoopInterfaces.IsEmpty())
		{
			for (const TScriptInterface<IFlecsGameLoopInterface>& GameLoopInterface : InGameLoopInterfaces)
			{
				WorldSettings.GameLoops.AddUnique(GameLoopInterface.GetObject());
			}
		}
		else if (bInUseDefaultGameLoop)
		{
			WorldSettings.GameLoops.AddUnique(NewObject<UFlecsDefaultGameLoop>(WorldSubsystem));
		}

		if (!InTickFunctions.IsEmpty())
		{
			WorldSettings.TickFunctions = InTickFunctions;
		}

		FlecsWorld = WorldSubsystem->CreateWorld("TestWorld", WorldSettings);
		FlecsWorld->AddToRoot();

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
			FlecsWorld->RemoveFromRoot();
			FlecsWorld = nullptr;
		}

		if (WorldSubsystem)
		{
			WorldSubsystem->RemoveFromRoot();
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

		if (StandalonePlayerController)
		{
			StandalonePlayerController = nullptr;
		}

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

	NO_DISCARD FORCEINLINE UFlecsWorldSubsystem* GetWorldSubsystem() const
	{
		return WorldSubsystem;
	}

	NO_DISCARD FORCEINLINE UWorld* GetTestWorld() const
	{
		return TestWorld.Get();
	}

}; // class FFlecsTestFixture

template<typename TDerived, typename TAsserter>
struct TFlecsWorldTest : TTest<TDerived, TAsserter>
{
	using Super = TTest<TDerived, TAsserter>;

	virtual void Setup() override
	{
		Fixture.SetUp({}, {}, WorldType(), ShouldUseDefaultGameLoop());
		OnWorldSetUp();
	}

	virtual void TearDown() override
	{
		OnWorldTearDown();
		Fixture.TearDown();
	}

protected:
	virtual EWorldType::Type WorldType() const
	{
		return EWorldType::GameRPC;
	}

	virtual bool ShouldUseDefaultGameLoop() const
	{
		return true;
	}

	virtual void OnWorldSetUp()
	{
	}

	virtual void OnWorldTearDown()
	{
	}

	NO_DISCARD UFlecsWorld* World() const
	{
		return Fixture.GetFlecsWorld();
	}

	NO_DISCARD UFlecsWorldSubsystem* WorldSubsystem() const
	{
		return Fixture.GetWorldSubsystem();
	}

	NO_DISCARD UWorld* UnrealWorld() const
	{
		return Fixture.GetTestWorld();
	}

	void TickWorld(const double InDeltaSeconds = 1.0 / 60.0) const
	{
		Fixture.TickWorld(InDeltaSeconds);
	}

private:
	FFlecsTestFixture Fixture;
}; // struct TFlecsWorldTest

#define FLECS_TEST_CLASS_WITH_FLAGS(_ClassName, _TestDir, _Flags) \
	TEST_CLASS_WITH_BASE_AND_FLAGS(_ClassName, _TestDir, TFlecsWorldTest, _Flags)

#define FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(_ClassName, _TestDir, _Flags, _TestTags) \
	TEST_CLASS_WITH_BASE_AND_FLAGS_AND_TAGS(_ClassName, _TestDir, TFlecsWorldTest, _Flags, _TestTags)

#endif // #if WITH_AUTOMATION_TESTS
