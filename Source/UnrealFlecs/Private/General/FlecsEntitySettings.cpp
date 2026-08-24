// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "General/FlecsEntitySettings.h"

#include "General/FlecsObjectRegistrationProviderBase.h"
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
	
	for (const UFlecsObjectRegistrationProviderBase* Provider : UFlecsObjectRegistrationProviderBase::IterateProviders())
	{
		for (const TSubclassOf<UObject>& ClassToRegister : Provider->GetClassesToRegister(false))
		{
			const TSolidNotNull<IFlecsObjectRegistrationInterface*> RegistrationInterface
				= CastChecked<IFlecsObjectRegistrationInterface>(ClassToRegister->GetDefaultObject());
			
#if WITH_EDITORONLY_DATA
			if (!RegistrationInterface->ShouldShowInSettings())
			{
				return;
			}
#endif // WITH_EDITORONLY_DATA
			
			CDOs.Add(CastChecked<UObject>(RegistrationInterface));
		}
	}

	CDOs.Sort([](const UObject& LHS, const UObject& RHS) -> bool
	{
		return GetNameSafe(LHS.GetClass()).Compare(GetNameSafe(RHS.GetClass())) < 0;
	});
}
