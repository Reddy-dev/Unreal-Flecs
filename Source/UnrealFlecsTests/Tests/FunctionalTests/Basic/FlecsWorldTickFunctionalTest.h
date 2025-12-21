// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Fixtures/FunctionalTesting/FlecsFunctionalTickBase.h"

#include "FlecsWorldTickFunctionalTest.generated.h"

UCLASS(BlueprintType)
class UNREALFLECSTESTS_API AFlecsWorldTickFunctionalTest : public AFlecsFunctionalTickBase
{
	GENERATED_BODY()

public:
	AFlecsWorldTickFunctionalTest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PrepareTest() override;
	virtual void StartTest() override;
	
	virtual void TickWithFlecs(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Flecs World Tick Functional Test")
	uint32 TargetTickCount = 10;
	
}; // class AFlecsWorldTickFunctionalTest
