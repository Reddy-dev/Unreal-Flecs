// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"

#include "Versioning/SolidVersioningTypes.h"

#include "FlecsGameLoopInterface.h"

#include "FlecsGameLoopObject.generated.h"

START_SOLID_ASSET_VERSION(UFlecsGameLoopObject)

END_SOLID_ASSET_VERSION() // UFlecsGameLoopObject

UCLASS(Abstract, EditInlineNew, BlueprintType, NotBlueprintable, Category = "Flecs|GameLoop")
class UNREALFLECS_API UFlecsGameLoopObject : public UObject, public IFlecsGameLoopInterface
{
	GENERATED_BODY()

public:
	UFlecsGameLoopObject();
	UFlecsGameLoopObject(const FObjectInitializer& ObjectInitializer);
	
}; // class UFlecsGameLoopObject
