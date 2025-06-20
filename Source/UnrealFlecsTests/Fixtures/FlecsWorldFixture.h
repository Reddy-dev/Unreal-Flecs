﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/Object.h"
#include "Worlds/FlecsWorldSubsystem.h"

#define FLECS_LOAD_OBJECT(AssetType, AssetPath) \
	StaticLoadObject(AssetType::StaticClass(), nullptr, AssetPath, nullptr, LOAD_None, nullptr)

class UNREALFLECSTESTS_API FFlecsTestFixture
{
public:
	TStrongObjectPtr<UWorld> TestWorld = nullptr;
	TStrongObjectPtr<UFlecsWorldSubsystem> WorldSubsystem = nullptr;
	TStrongObjectPtr<UFlecsWorld> FlecsWorld = nullptr;

	void SetUp(TScriptInterface<IFlecsGameLoopInterface> InGameLoopInterface = nullptr,
	           const TArray<UObject*>& InModules = {})
	{
		TestWorld = TStrongObjectPtr(UWorld::CreateWorld(EWorldType::Game,
			false, TEXT("FlecsTestWorld")));
		
		check(TestWorld.IsValid());

		FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
		WorldContext.SetCurrentWorld(TestWorld.Get());

		WorldSubsystem = TStrongObjectPtr(TestWorld->GetSubsystem<UFlecsWorldSubsystem>());
		check(WorldSubsystem.IsValid());

		if (!InGameLoopInterface)
		{
			InGameLoopInterface = NewObject<UFlecsDefaultGameLoop>(WorldSubsystem.Get());
		}

		// Create world settings
		FFlecsWorldSettingsInfo WorldSettings;
		WorldSettings.WorldName = TEXT("TestWorld");
		WorldSettings.GameLoop = InGameLoopInterface.GetObject();
		WorldSettings.Modules = InModules;

		FlecsWorld = TStrongObjectPtr(WorldSubsystem->CreateWorld(TEXT("TestWorld"), WorldSettings));

		TestWorld->InitializeActorsForPlay(FURL());
		TestWorld->BeginPlay();
	}

	void TearDown()
	{
		if (TestWorld.IsValid())
		{
			TestWorld->EndPlay(EEndPlayReason::Quit);
			TestWorld->DestroyWorld(false);
			GEngine->DestroyWorldContext(TestWorld.Get());
		}

		TestWorld.Reset();
		WorldSubsystem.Reset();
		FlecsWorld.Reset();
	}
	
}; // class FFlecsTestFixture

#define FLECS_FIXTURE_LIFECYCLE(FixtureName) \
	BeforeEach([this]() \
	{ \
		FixtureName.SetUp(); \
	}); \
	AfterEach([this]() \
	{ \
		FixtureName.TearDown(); \
	})
