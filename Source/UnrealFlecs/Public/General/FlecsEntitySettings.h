// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsModuleSettings.h"

#include "FlecsEntitySettings.generated.h"

class UFlecsSystemObject;
class UPackage;

/**
 * 
 */
UCLASS(Config = Flecs, DefaultConfig, DisplayName = "Flecs Entity Settings")
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
	TArray<TObjectPtr<UObject>> CDOs;

protected:
	void OnPostEngineInit();
	void OnModulePackagesUnloaded(TConstArrayView<UPackage*>);

private:
	bool bEngineInitialized = false;
	
}; // class UFlecsEntitySettings
