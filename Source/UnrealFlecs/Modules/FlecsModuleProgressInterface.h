﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FlecsModuleProgressInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(meta = (CannotImplementInterfaceInBlueprint))
class UNREALFLECS_API UFlecsModuleProgressInterface : public UInterface
{
	GENERATED_BODY()
}; // class UFlecsModuleProgressInterface

class UNREALFLECS_API IFlecsModuleProgressInterface
{
	GENERATED_BODY()
public:
	virtual void ProgressModule(double InDeltaTime) PURE_VIRTUAL(IFlecsModuleProgressInterface::ProgressModule, );

}; // class IFlecsModuleProgressInterface
