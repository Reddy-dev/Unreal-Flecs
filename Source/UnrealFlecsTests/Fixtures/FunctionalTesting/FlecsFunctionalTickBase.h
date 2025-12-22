// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsFunctionTestBase.h"

#include "Entities/FlecsEntityHandle.h"

#include "FlecsFunctionalTickBase.generated.h"

UCLASS(Abstract, BlueprintType)
class UNREALFLECSTESTS_API AFlecsFunctionalTickBase : public AFlecsFunctionTestBase
{
	GENERATED_BODY()

public:
	AFlecsFunctionalTickBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PrepareTest() override;
	virtual void StartTest() override;
	virtual void FinishTest(EFunctionalTestResult TestResult, const FString& Message) override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void TickWithFlecs(float DeltaTime);

protected:
	bool bStartedFlecsTicking = false;
	bool bCountersResetAfterFlecsStart = false;

	uint32 FunctionalTestTickCount = 0;

	uint32 MainLoopCounter = 0;
	uint32 PrePhysicsCounter = 0;
	uint32 DuringPhysicsCounter = 0;
	uint32 PostPhysicsCounter = 0;
	uint32 PostUpdateWorkCounter = 0;

	FFlecsEntityHandle MainLoopSystem;
	FFlecsEntityHandle PrePhysicsSystem;
	FFlecsEntityHandle DuringPhysicsSystem;
	FFlecsEntityHandle PostPhysicsSystem;
	FFlecsEntityHandle PostUpdateWorkSystem;
	
}; // class AFlecsFunctionalTickBase
