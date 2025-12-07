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

namespace Unreal::Flecs::Testing::impl
{
	static const FString DefaultTags = TEXT("");
} // namespace Unreal::Flecs::Testing::impl

class UNREALFLECSTESTS_API FFlecsTestFixture
{
public:
	TUniquePtr<FTestWorldWrapper> TestWorldWrapper;
	
	TWeakObjectPtr<UWorld> TestWorld;
	UFlecsWorldSubsystem* WorldSubsystem = nullptr;
	UFlecsWorld* FlecsWorld = nullptr;
	TSharedPtr<FScopedTestEnvironment> ScopedTestEnvironment = nullptr;

	// @TODO: add test support for multiple game loops
	void SetUp(TArray<TScriptInterface<IFlecsGameLoopInterface>> InGameLoopInterfaces = {}, TArray<FFlecsTickFunctionSettingsInfo> InTickFunctions = {},
	           const TArray<UObject*>& InModules = {})
	{
		ScopedTestEnvironment = FScopedTestEnvironment::Get();
		
		TestWorldWrapper = MakeUnique<FTestWorldWrapper>();
		TestWorldWrapper->CreateTestWorld(EWorldType::Game);

		TestWorld = TestWorldWrapper->GetTestWorld();

		WorldSubsystem = TestWorld->GetSubsystem<UFlecsWorldSubsystem>();
		check(IsValid(WorldSubsystem));

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

	void TickWorld(const double DeltaTime = 0.016) const
	{
		if (DeltaTime == 0.0)
		{
			TestWorldWrapper->GetTestWorld()->Tick(LEVELTICK_PauseTick, 0.0f);

			if (TestWorld->HasBegunPlay())
			{
				// If this has begin play increment the global counter to emulate gameplay
				GFrameCounter++;
			}
		}
		else
		{
			TestWorldWrapper->TickTestWorld(DeltaTime);
		}
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

		TestWorldWrapper->DestroyTestWorld(true);
		TestWorldWrapper = nullptr;

		ScopedTestEnvironment = nullptr;
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

#define FLECS_FIXTURE_LIFECYCLE_LATENT(FixtureName) \
	LatentBeforeEach([this](const FDoneDelegate& Done) \
	{ \
		FixtureName.SetUp(); \
		Done.Execute(); \
	}); \
	LatentAfterEach([this](const FDoneDelegate& Done) \
	{ \
		FixtureName.TearDown(); \
		Done.Execute(); \
	});

#define xTEST_METHOD_WITH_TAGS(_MethodName, _TestTags) \
	void _MethodName()

#define xTEST_METHOD(_MethodName) xTEST_METHOD_WITH_TAGS(_MethodName, Unreal::Flecs::Testing::impl::DefaultTags)

#endif // #if WITH_AUTOMATION_TESTS
