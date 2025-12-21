// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FunctionalTest.h"

#include "FlecsGamePauseFunctionalTest.generated.h"

UCLASS()
class UNREALFLECSTESTS_API AFlecsGamePauseFunctionalTest : public AFunctionalTest
{
	GENERATED_BODY()

public:
	AFlecsGamePauseFunctionalTest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void PrepareTest() override;
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
}; // class AFlecsGamePauseFunctionalTest
