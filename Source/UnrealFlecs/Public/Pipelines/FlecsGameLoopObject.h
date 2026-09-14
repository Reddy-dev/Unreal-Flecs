// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "UObject/Object.h"

#include "Versioning/SolidVersioningTypes.h"

#include "FlecsGameLoopInterface.h"
#include "Worlds/Settings/FlecsWorldInfoSettings.h"

#include "FlecsGameLoopObject.generated.h"

START_SOLID_ASSET_VERSION(UFlecsGameLoopObject)

END_SOLID_ASSET_VERSION() // UFlecsGameLoopObject

// @TODO: make compatible with flecs modules maybe?

UCLASS(Abstract, EditInlineNew, BlueprintType, NotBlueprintable, Category = "Flecs|GameLoop")
class UNREALFLECS_API UFlecsGameLoopObject : public UObject, public IFlecsGameLoopInterface
{
	GENERATED_BODY()

public:
	UFlecsGameLoopObject();
	UFlecsGameLoopObject(const FObjectInitializer& ObjectInitializer);
	
	virtual void DeinitializeGameLoop(TSolidNotNull<UFlecsWorld*> InWorld, const FFlecsEntityHandle& InGameLoopEntity) override;
	
	UPROPERTY(EditAnywhere, Category = "Flecs | GameLoop")
	FFlecsTickFunctionSettingsInfo TickFunctionSettings;
	
	virtual TSharedStruct<FFlecsTickFunction> InitializeTickFunction(TSolidNotNull<UFlecsWorld*> InWorld) override;
	
	virtual NO_DISCARD TSharedStruct<FFlecsTickFunction> GetTickFunction() const override;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | GameLoop")
	UFlecsWorld* GetFlecsWorld() const;
	
	NO_DISCARD TSolidNotNull<UFlecsWorld*> GetFlecsWorldChecked() const;
	
protected:
	TSharedStruct<FFlecsTickFunction> TickFunction;


}; // class UFlecsGameLoopObject
