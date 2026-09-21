// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "FlecsRegisteredObjectDependencyTestTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsRegisteredObjectDependencyTestTypes)

TArray<TSubclassOf<UObject>> UFlecsRegisteredObjectCycleATestObject::GetDependentRegistrationClasses() const
{
	return { UFlecsRegisteredObjectCycleBTestObject::StaticClass() };
}

TArray<TSubclassOf<UObject>> UFlecsRegisteredObjectCycleBTestObject::GetDependentRegistrationClasses() const
{
	return { UFlecsRegisteredObjectCycleATestObject::StaticClass() };
}
