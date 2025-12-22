// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsFunctionalTestBase.h"

#include "Worlds/FlecsWorldSubsystem.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsFunctionalTestBase)

AFlecsFunctionalTestBase::AFlecsFunctionalTestBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	TestTags += "[Flecs]";
}

void AFlecsFunctionalTestBase::PrepareTest()
{
	Super::PrepareTest();

	OwningFlecsWorld = UFlecsWorldSubsystem::GetDefaultWorldStatic(this);

	if UNLIKELY_IF(!OwningFlecsWorld.IsValid())
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("FlecsWorld is not valid"));
	}
}

UFlecsWorld* AFlecsFunctionalTestBase::GetOwningFlecsWorld() const
{
	return OwningFlecsWorld.Get();
}

TSolidNotNull<UFlecsWorld*> AFlecsFunctionalTestBase::GetOwningFlecsWorldChecked() const
{
	solid_checkf(OwningFlecsWorld.IsValid(), TEXT("FlecsWorld is not valid!"));
	return OwningFlecsWorld.Get();
}
