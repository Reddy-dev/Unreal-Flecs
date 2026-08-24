// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "FlecsModuleSettings.h"

#include "FlecsEntitySettings.generated.h"

class UFlecsSystemObject;
class UPackage;

/**
 * 
 */
UCLASS(Config = Flecs, DefaultConfig, DisplayName = "Flecs Object Registration Instance Settings")
class UNREALFLECS_API UFlecsEntitySettings : public UFlecsModuleSettings
{
	GENERATED_BODY()

public:
	UFlecsEntitySettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PostInitProperties() override;
	virtual void BeginDestroy() override;

	/** Rebuilds the editor-visible catalog of concrete Flecs system CDOs. */
	void BuildSystemList();

	UPROPERTY(VisibleAnywhere, Category = "Flecs", Transient, Instanced, EditFixedSize, meta = (EditInline))
	TArray<TObjectPtr<const UObject>> CDOs;

protected:
	void OnPostEngineInit();
	void OnModulePackagesUnloaded(TConstArrayView<UPackage*> InUnloadedPackages);

private:
	bool bEngineInitialized = false;
	
}; // class UFlecsEntitySettings
