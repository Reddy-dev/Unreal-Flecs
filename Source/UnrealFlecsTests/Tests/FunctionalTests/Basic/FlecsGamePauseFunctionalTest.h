// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Fixtures/FunctionalTesting/FlecsFunctionalTickBase.h"

#include "SolidMacros/Macros.h"

#include "FlecsGamePauseFunctionalTest.generated.h"

UENUM(BlueprintType)
enum class EFlecsGamePauseFunctionalTestConfig : uint8
{
	UWorldToFlecsWorldSync,
	FlecsWorldToUWorldSync,
}; // enum class EFlecsGamePauseFunctionalTestState

enum class EFlecsGamePauseFunctionalTestState : uint8
{
	RunningBeforePause,
	Paused,
	RunningAfterPause,
	Completed,
}; // enum class EFlecsGamePauseFunctionalTestState

UCLASS()
class UNREALFLECSTESTS_API AFlecsGamePauseFunctionalTest : public AFlecsFunctionalTickBase
{
	GENERATED_BODY()

public:
	AFlecsGamePauseFunctionalTest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PrepareTest() override;
	virtual void StartTest() override;

	virtual void TickWithFlecs(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Flecs Functional Test")
	EFlecsGamePauseFunctionalTestConfig PauseMethod = EFlecsGamePauseFunctionalTestConfig::UWorldToFlecsWorldSync;

	UPROPERTY(EditAnywhere, Category = "Flecs Functional Test")
	EFlecsGamePauseFunctionalTestConfig	ResumeMethod = EFlecsGamePauseFunctionalTestConfig::UWorldToFlecsWorldSync;

	UPROPERTY(EditAnywhere, Category = "Flecs Functional Test")
	uint32 TargetTickCountBeforePause = 5;

	UPROPERTY(EditAnywhere, Category = "Flecs Functional Test")
	uint32 PauseDurationInTicks = 5;

	UPROPERTY(EditAnywhere, Category = "Flecs Functional Test")
	uint32 TargetTickCountAfterPause = 5;

protected:
	NO_DISCARD bool IsWorldPaused() const;

	void PauseWorld() const;
	void ResumeWorld() const;

	void SetState(const EFlecsGamePauseFunctionalTestState NewState);

	EFlecsGamePauseFunctionalTestState CurrentState = EFlecsGamePauseFunctionalTestState::RunningBeforePause;
	
}; // class AFlecsGamePauseFunctionalTest
