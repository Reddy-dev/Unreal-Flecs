// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsGamePauseFunctionalTest.h"

#include "Kismet/GameplayStatics.h"

#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsGamePauseFunctionalTest)

AFlecsGamePauseFunctionalTest::AFlecsGamePauseFunctionalTest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
}

void AFlecsGamePauseFunctionalTest::PrepareTest()
{
	Super::PrepareTest();

	CurrentState = EFlecsGamePauseFunctionalTestState::RunningBeforePause;
}

void AFlecsGamePauseFunctionalTest::StartTest()
{
	Super::StartTest();
}

void AFlecsGamePauseFunctionalTest::TickWithFlecs(float DeltaTime)
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

	if (CurrentState == EFlecsGamePauseFunctionalTestState::RunningBeforePause)
	{
		PauseWorld();
		SetState(EFlecsGamePauseFunctionalTestState::Paused);
	}
	else if (CurrentState == EFlecsGamePauseFunctionalTestState::Paused)
	{
		if (IsWorldPaused())
		{
			AssertTrue(DeltaTime == 0.0f, TEXT("DeltaTime should be zero when the world is paused."));
			
			ResumeWorld();
			SetState(EFlecsGamePauseFunctionalTestState::RunningAfterPause);
		}
		else
		{
			AddError(TEXT("World did not pause as expected."));
			FinishTest(EFunctionalTestResult::Failed, TEXT("World did not pause as expected."));
		}
	}
	else if (CurrentState == EFlecsGamePauseFunctionalTestState::RunningAfterPause)
	{
		FinishTest(EFunctionalTestResult::Succeeded, TEXT("World paused and resumed successfully."));
	}
}

bool AFlecsGamePauseFunctionalTest::IsWorldPaused() const
{
	return UGameplayStatics::IsGamePaused(GetWorld());
}

void AFlecsGamePauseFunctionalTest::PauseWorld() const
{
	if (PauseMethod == EFlecsGamePauseFunctionalTestConfig::UWorldToFlecsWorldSync)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
	else if (PauseMethod == EFlecsGamePauseFunctionalTestConfig::FlecsWorldToUWorldSync)
	{
		GetOwningFlecsWorldChecked()->SetTimeScale(0.0f);
	}
}

void AFlecsGamePauseFunctionalTest::ResumeWorld() const
{
	if (PauseMethod == EFlecsGamePauseFunctionalTestConfig::UWorldToFlecsWorldSync)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}
	else if (PauseMethod == EFlecsGamePauseFunctionalTestConfig::FlecsWorldToUWorldSync)
	{
		GetOwningFlecsWorldChecked()->SetTimeScale(1.0f);
	}
}

void AFlecsGamePauseFunctionalTest::SetState(const EFlecsGamePauseFunctionalTestState NewState)
{
	CurrentState = NewState;
}
