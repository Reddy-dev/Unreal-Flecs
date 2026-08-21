// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "General/FlecsEntitySettings.h"

#include "Misc/CoreDelegates.h"
#include "UObject/UObjectHash.h"
#include "UObject/UObjectGlobals.h"

#include "Systems/FlecsSystemObject.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsEntitySettings)

UFlecsEntitySettings::UFlecsEntitySettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FCoreDelegates::GetOnPostEngineInit().AddUObject(this, &UFlecsEntitySettings::OnPostEngineInit);
	
	FCoreUObjectDelegates::CompiledInUObjectsRemovedDelegate.AddUObject(
		this, &UFlecsEntitySettings::OnModulePackagesUnloaded);
}

void UFlecsEntitySettings::PostInitProperties()
{
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		BuildSystemList();
	}
}

void UFlecsEntitySettings::BeginDestroy()
{
	FCoreDelegates::GetOnPostEngineInit().RemoveAll(this);
	FCoreUObjectDelegates::CompiledInUObjectsRemovedDelegate.RemoveAll(this);

	Super::BeginDestroy();
}

void UFlecsEntitySettings::OnPostEngineInit()
{
	bEngineInitialized = true;
	BuildSystemList();
}

void UFlecsEntitySettings::OnModulePackagesUnloaded(MAYBE_UNUSED TConstArrayView<UPackage*> InUnloadedPackages)
{
	if (bEngineInitialized)
	{
		BuildSystemList();
	}
}

void UFlecsEntitySettings::BuildSystemList()
{
	CDOs.Reset();

	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* ObjectClass = *It;

		if (!IsValid(ObjectClass) ||
			ObjectClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
		{
			continue;
		}
		
		if (!ObjectClass->ImplementsInterface(UFlecsObjectRegistrationInterface::StaticClass()))
		{
			continue;
		}

		UObject* CDO = GetMutableDefault<UObject>(ObjectClass);

		if UNLIKELY_IF(!IsValid(CDO))
		{
			continue;
		}

		const IFlecsObjectRegistrationInterface* RegistrationInterface = Cast<IFlecsObjectRegistrationInterface>(CDO);

#if WITH_EDITORONLY_DATA
		if (!RegistrationInterface->ShouldShowInSettings())
		{
			continue;
		}
#endif // WITH_EDITORONLY_DATA

		CDOs.Add(CDO);
	}

	CDOs.Sort([](const UObject& LHS, const UObject& RHS) -> bool
	{
		return GetNameSafe(LHS.GetClass()).Compare(GetNameSafe(RHS.GetClass())) < 0;
	});
}
