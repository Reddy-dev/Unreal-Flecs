// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FunctionalTest.h"

#include "FlecsFunctionalTickBase.generated.h"

UCLASS(Abstract, BlueprintType)
class UNREALFLECSTESTS_API AFlecsFunctionalTickBase : public AFunctionalTest
{
	GENERATED_BODY()

public:
	AFlecsFunctionalTickBase(const FObjectInitializer& ObjectInitializer);

	virtual void PrepareTest() override;
	virtual void StartTest() override;

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
	
}; // class AFlecsFunctionalTickBase
