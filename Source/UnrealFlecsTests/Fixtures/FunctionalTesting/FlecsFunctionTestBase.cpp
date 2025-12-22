// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsFunctionTestBase.h"

#include "Worlds/FlecsWorldSubsystem.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsFunctionTestBase)

AFlecsFunctionTestBase::AFlecsFunctionTestBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFlecsFunctionTestBase::PrepareTest()
{
	Super::PrepareTest();

	OwningFlecsWorld = UFlecsWorldSubsystem::GetDefaultWorldStatic(this);

	if UNLIKELY_IF(!OwningFlecsWorld.IsValid())
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("FlecsWorld is not valid"));
	}
}

UFlecsWorld* AFlecsFunctionTestBase::GetOwningFlecsWorld() const
{
	return OwningFlecsWorld.Get();
}

TSolidNotNull<UFlecsWorld*> AFlecsFunctionTestBase::GetOwningFlecsWorldChecked() const
{
	solid_checkf(OwningFlecsWorld.IsValid(), TEXT("FlecsWorld is not valid!"));
	return OwningFlecsWorld.Get();
}
