// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameFeatureAction.h"

#include "GameFeatureAction_ImportFlecsModules.generated.h"

class UFlecsModuleSetDataAsset;

UCLASS(BlueprintType, meta = (DisplayName = "Import Flecs Modules"))
class UNREALFLECS_API UGameFeatureAction_ImportFlecsModules : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	UGameFeatureAction_ImportFlecsModules();
	
	UPROPERTY(EditAnywhere, Instanced, Category = "Modules",
		meta = (ObjectMustImplement = "/Script/UnrealFlecs.FlecsModuleInterface", NoElementDuplicate))
	TArray<TObjectPtr<UObject>> Modules;

	UPROPERTY(EditAnywhere, Category = "Modules", meta = (NoElementDuplicate))
	TArray<TObjectPtr<UFlecsModuleSetDataAsset>> ModuleSets;
	
}; // class UGameFeatureAction_ImportFlecsModules
