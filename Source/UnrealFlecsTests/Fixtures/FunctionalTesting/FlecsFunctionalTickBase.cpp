// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsFunctionalTickBase.h"

#include "Pipelines/TickFunctions/FlecsTickTypeNativeTags.h"
#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsFunctionalTickBase)

AFlecsFunctionalTickBase::AFlecsFunctionalTickBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	PrimaryActorTick.TickGroup = TG_LastDemotable;
}

void AFlecsFunctionalTickBase::PrepareTest()
{
	Super::PrepareTest();

	MainLoopCounter = 0;
	PrePhysicsCounter = 0;
	DuringPhysicsCounter = 0;
	PostPhysicsCounter = 0;
	PostUpdateWorkCounter = 0;
	FunctionalTestTickCount = 0;
	bStartedFlecsTicking = false;
	bCountersResetAfterFlecsStart = false;
}

void AFlecsFunctionalTickBase::StartTest()
{
	Super::StartTest();

	const TSolidNotNull<const UFlecsWorld*> FlecsWorld = GetOwningFlecsWorldChecked();

	MainLoopSystem = FlecsWorld->World.system<>()
				.kind(flecs::OnUpdate)
				.each([this](flecs::iter& Iter, size_t Index)
				{
					if (!bIsRunning)
					{
						return;
					}
					
					bStartedFlecsTicking = true;
					MainLoopCounter++;
				});
	//.add(FlecsWorld->GetTagEntity(FlecsTickType_MainLoop).GetFlecsId());
	
	PrePhysicsSystem = FlecsWorld->World.system<>()
		.kind(flecs::OnUpdate)
		.each([this](flecs::iter& Iter, size_t Index)
		{
			if (!bIsRunning)
			{
				return;
			}
			
			PrePhysicsCounter++;
		})
		.add(FlecsWorld->GetTagEntity(FlecsTickType_PrePhysics).GetFlecsId());
	
	DuringPhysicsSystem = FlecsWorld->World.system<>()
		.kind(flecs::OnUpdate)
		.each([this](flecs::iter& Iter, size_t Index)
		{
			if (!bIsRunning)
			{
				return;
			}
			
			DuringPhysicsCounter++;
		})
		.add(FlecsWorld->GetTagEntity(FlecsTickType_DuringPhysics).GetFlecsId());
	
	PostPhysicsSystem = FlecsWorld->World.system<>()
		.kind(flecs::OnUpdate)
		.each([this](flecs::iter& Iter, size_t Index)
		{
			if (!bIsRunning)
			{
				return;
			}
			
			PostPhysicsCounter++;
		})
		.add(FlecsWorld->GetTagEntity(FlecsTickType_PostPhysics).GetFlecsId());
	
	PostUpdateWorkSystem = FlecsWorld->World.system<>()
		.kind(flecs::OnUpdate)
		.each([this](flecs::iter& Iter, size_t Index)
		{
			if (!bIsRunning)
			{
				return;
			}
			
			PostUpdateWorkCounter++;
		})
		.add(FlecsWorld->GetTagEntity(FlecsTickType_PostUpdateWork).GetFlecsId());
}

void AFlecsFunctionalTickBase::FinishTest(EFunctionalTestResult TestResult, const FString& Message)
{
	Super::FinishTest(TestResult, Message);

	if (MainLoopSystem.IsValid())
	{
		MainLoopSystem.Destroy();
	}

	if (PrePhysicsSystem.IsValid())
	{
		PrePhysicsSystem.Destroy();
	}

	if (DuringPhysicsSystem.IsValid())
	{
		DuringPhysicsSystem.Destroy();
	}

	if (PostPhysicsSystem.IsValid())
	{
		PostPhysicsSystem.Destroy();
	}

	if (PostUpdateWorkSystem.IsValid())
	{
		PostUpdateWorkSystem.Destroy();
	}
}

void AFlecsFunctionalTickBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsRunning)
	{
		return;
	}

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

	TickWithFlecs(DeltaSeconds);
}

void AFlecsFunctionalTickBase::TickWithFlecs(float DeltaTime)
{
}
