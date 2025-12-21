// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsWorldTickFunctionalTest.h"

#include "Pipelines/TickFunctions/FlecsTickTypeNativeTags.h"
#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsWorldTickFunctionalTest)

AFlecsWorldTickFunctionalTest::AFlecsWorldTickFunctionalTest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	PrimaryActorTick.TickGroup = TG_LastDemotable;
}

void AFlecsWorldTickFunctionalTest::PrepareTest()
{
	Super::PrepareTest();

	const UFlecsWorld* FlecsWorld = UFlecsWorldSubsystem::GetDefaultWorldStatic(this);

	MainLoopCounter = 0;
	PrePhysicsCounter = 0;
	DuringPhysicsCounter = 0;
	PostPhysicsCounter = 0;
	PostUpdateWorkCounter = 0;
	FunctionalTestTickCount = 0;
	bStartedFlecsTicking = false;
	bCountersResetAfterFlecsStart = false;

	if UNLIKELY_IF(!IsValid(FlecsWorld))
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("FlecsWorld is not valid!"));
		return;
	}

	FFlecsEntityHandle MainLoopSystem = FlecsWorld->World.system<>()
				.kind(flecs::OnUpdate)
				.each([this](flecs::iter& Iter, size_t Index)
				{
					bStartedFlecsTicking = true;
					MainLoopCounter++;
				});
	//.add(FlecsWorld->GetTagEntity(FlecsTickType_MainLoop).GetFlecsId());
	
	FFlecsEntityHandle PrePhysicsSystem = FlecsWorld->World.system<>()
		.kind(flecs::OnUpdate)
		.each([this](flecs::iter& Iter, size_t Index)
		{
			PrePhysicsCounter++;
		})
		.add(FlecsWorld->GetTagEntity(FlecsTickType_PrePhysics).GetFlecsId());
	
	FFlecsEntityHandle DuringPhysicsSystem = FlecsWorld->World.system<>()
		.kind(flecs::OnUpdate)
		.each([this](flecs::iter& Iter, size_t Index)
		{
			DuringPhysicsCounter++;
		})
		.add(FlecsWorld->GetTagEntity(FlecsTickType_DuringPhysics).GetFlecsId());
	
	FFlecsEntityHandle PostPhysicsSystem = FlecsWorld->World.system<>()
		.kind(flecs::OnUpdate)
		.each([this](flecs::iter& Iter, size_t Index)
		{
			PostPhysicsCounter++;
		})
		.add(FlecsWorld->GetTagEntity(FlecsTickType_PostPhysics).GetFlecsId());
	
	FFlecsEntityHandle PostUpdateWorkSystem = FlecsWorld->World.system<>()
		.kind(flecs::OnUpdate)
		.each([this](flecs::iter& Iter, size_t Index)
		{
			PostUpdateWorkCounter++;
		})
		.add(FlecsWorld->GetTagEntity(FlecsTickType_PostUpdateWork).GetFlecsId());
}

void AFlecsWorldTickFunctionalTest::StartTest()
{
	Super::StartTest();
	
}

void AFlecsWorldTickFunctionalTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bStartedFlecsTicking)
	{
		// Wait until Flecs starts ticking
		return;
	}

	if (!bCountersResetAfterFlecsStart)
	{
		MainLoopCounter = 0;
		PrePhysicsCounter = 0;
		DuringPhysicsCounter = 0;
		PostPhysicsCounter = 0;
		PostUpdateWorkCounter = 0;

		FunctionalTestTickCount = 0;

		bCountersResetAfterFlecsStart = true;
		
		return;
	}

	++FunctionalTestTickCount;

	if (FunctionalTestTickCount > 0)
	{
		if (FunctionalTestTickCount != MainLoopCounter)
		{
			AddError(
				FString::Printf(TEXT("MainLoopCounter (%d) did not match FunctionalTestTickCount (%d)"),
					MainLoopCounter, FunctionalTestTickCount));
		}

		if (FunctionalTestTickCount != PrePhysicsCounter)
		{
			AddError(
				FString::Printf(TEXT("PrePhysicsCounter (%d) did not match FunctionalTestTickCount (%d)"),
					PrePhysicsCounter, FunctionalTestTickCount));
		}

		if (FunctionalTestTickCount != DuringPhysicsCounter)
		{
			AddError(
				FString::Printf(TEXT("DuringPhysicsCounter (%d) did not match FunctionalTestTickCount (%d)"),
					DuringPhysicsCounter, FunctionalTestTickCount));
		}

		if (FunctionalTestTickCount != PostPhysicsCounter)
		{
			AddError(
				FString::Printf(TEXT("PostPhysicsCounter (%d) did not match FunctionalTestTickCount (%d)"),
					PostPhysicsCounter, FunctionalTestTickCount));
		}

		if (FunctionalTestTickCount != PostUpdateWorkCounter)
		{
			AddError(
				FString::Printf(TEXT("PostUpdateWorkCounter (%d) did not match FunctionalTestTickCount (%d)"),
					(PostUpdateWorkCounter), FunctionalTestTickCount));
		}

		if (FunctionalTestTickCount >= TargetTickCount)
		{
			FinishTest(EFunctionalTestResult::Default, TEXT(""));
		}
	}
}

