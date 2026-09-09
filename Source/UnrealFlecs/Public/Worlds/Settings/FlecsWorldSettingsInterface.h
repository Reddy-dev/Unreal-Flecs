// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/Interface.h"

#include "SolidMacros/Macros.h"

#include "FlecsWorldSettingsInterface.generated.h"

class UFlecsWorldSettingsAsset;

// This class does not need to be modified.
UINTERFACE()
class UFlecsWorldSettingsInterface : public UInterface
{
	GENERATED_BODY()
}; // class UFlecsWorldSettingsInterface

/**
 * 
 */
class UNREALFLECS_API IFlecsWorldSettingsInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual NO_DISCARD bool IsFlecsEnabled() const
		PURE_VIRTUAL(IFlecsWorldSettingsInterface::IsFlecsEnabled, return false;);
	
	virtual NO_DISCARD UFlecsWorldSettingsAsset* GetFlecsWorldSettingsAsset() const
		PURE_VIRTUAL(IFlecsWorldSettingsInterface::GetFlecsWorldSettingsAsset, return nullptr;);

}; // class IFlecsWorldSettingsInterface
