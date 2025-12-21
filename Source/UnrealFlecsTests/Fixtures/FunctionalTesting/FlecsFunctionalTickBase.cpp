// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsFunctionalTickBase.h"

#include "Entities/FlecsEntityHandle.h"
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
}

void AFlecsFunctionalTickBase::StartTest()
{
	Super::StartTest();

	const UFlecsWorld* FlecsWorld = UFlecsWorldSubsystem::GetDefaultWorldStatic(this);

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

void AFlecsFunctionalTickBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

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
