// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "FlecsWorldTickFunctionalTest.generated.h"

UCLASS(BlueprintType)
class UNREALFLECSTESTS_API AFlecsWorldTickFunctionalTest : public AFunctionalTest
{
	GENERATED_BODY()

public:
	AFlecsWorldTickFunctionalTest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PrepareTest() override;
	virtual void StartTest() override;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Flecs World Tick Functional Test")
	uint32 TargetTickCount = 10;

protected:
	bool bStartedFlecsTicking = false;
	bool bCountersResetAfterFlecsStart = false;

	uint32 FunctionalTestTickCount = 0;

	uint32 MainLoopCounter = 0;
	uint32 PrePhysicsCounter = 0;
	uint32 DuringPhysicsCounter = 0;
	uint32 PostPhysicsCounter = 0;
	uint32 PostUpdateWorkCounter = 0;
	
}; // class AFlecsWorldTickFunctionalTest
