// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"

#include "Types/SolidNotNull.h"

#include "FlecsObjectRegistrationNetworkFlags.h"
#include "FlecsObjectRegistrationStateType.h"
#include "UnrealFlecsRegistrationScopeType.h"

#include "FlecsObjectRegistrationInterface.generated.h"

class UFlecsWorldInterfaceObject;

// This class does not need to be modified.
UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UFlecsObjectRegistrationInterface : public UInterface
{
	GENERATED_BODY()
}; // class UFlecsObjectRegistrationInterface

class UNREALFLECS_API IFlecsObjectRegistrationInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void RegisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld);
	virtual void UnregisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld);
	virtual void FlecsWorldBeginPlay(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld);
	
	virtual void SetFlecsObjectState(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld, const EFlecsObjectRegistrationStateType InState);
	
	virtual NO_DISCARD EFlecsObjectRegistrationStateType GetObjectRegistrationState() const
	{
		return EFlecsObjectRegistrationStateType::Active;
	}
	
	// Impl must be safe to call on the CDO
	virtual NO_DISCARD bool ShouldAutoRegisterFromCDO() const { return true; }
	virtual NO_DISCARD bool ShouldAutoRegisterWithWorld(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InFlecsWorld) const { return true; }
	
	virtual NO_DISCARD EUnrealFlecsRegistrationScopeType GetRegistrationScopeType() const
	{
		return EUnrealFlecsRegistrationScopeType::Module;
	}
	
#if WITH_EDITORONLY_DATA
	virtual NO_DISCARD bool ShouldShowInSettings() const { return false; }
#endif // WITH_EDITORONLY_DATA
	
	virtual NO_DISCARD uint8 GetObjectRegistrationNetworkFlags() const
	{
		return static_cast<uint8>(EFlecsObjectRegistrationNetworkFlags::All);
	}
	
	/**
	 * Optional for Module and Plugin scopes. A Module scope defaults to this object's native module;
	 * a Plugin scope defaults to that module's owning Unreal plugin. Custom identifier scopes require a name.
	 */
	virtual NO_DISCARD FName GetScopeName() const { return NAME_None; }

}; // class IFlecsObjectRegistrationInterface
