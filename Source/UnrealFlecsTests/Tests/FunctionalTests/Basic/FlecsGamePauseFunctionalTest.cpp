// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsGamePauseFunctionalTest.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsGamePauseFunctionalTest)

AFlecsGamePauseFunctionalTest::AFlecsGamePauseFunctionalTest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
}

void AFlecsGamePauseFunctionalTest::PrepareTest()
{
	Super::PrepareTest();
}

void AFlecsGamePauseFunctionalTest::BeginPlay()
{
	Super::BeginPlay();

	
}

void AFlecsGamePauseFunctionalTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}

