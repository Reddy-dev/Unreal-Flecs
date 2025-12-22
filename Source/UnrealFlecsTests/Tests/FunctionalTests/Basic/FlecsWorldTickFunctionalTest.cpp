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

	PrimaryActorTick.TickGroup = TG_LastDemotable;
}

void AFlecsWorldTickFunctionalTest::PrepareTest()
{
	Super::PrepareTest();
}

void AFlecsWorldTickFunctionalTest::StartTest()
{
	Super::StartTest();
}

void AFlecsWorldTickFunctionalTest::TickWithFlecs(float DeltaTime)
{
	Super::TickWithFlecs(DeltaTime);

	if (!bIsRunning)
	{
		return;
	}

	if (FunctionalTestTickCount <= 0)
	{
		return;
	}

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

