// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"
#include "Misc/AutomationTest.h"

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

/**
 * Lifecycle and registration policy interface for UObject-backed Flecs objects.
 *
 * UFlecsWorld creates an instance for each eligible world and calls RegisterObject
 * during registration. FlecsWorldBeginPlay is called after the world enters
 * begin play; when an object is registered after that point, it is called as
 * part of the same registration operation. UnregisterObject is called before
 * the object is removed from the world.
 *
 * Automatic discovery and editor settings filters are evaluated on the class
 * default object (CDO). Those implementations must not depend on per-world
 * runtime state.
 */
class UNREALFLECS_API IFlecsObjectRegistrationInterface
{
	GENERATED_BODY()

public:
	/**
	 * Registers this object with a Flecs world.
	 *
	 * Called after the object has been created and accepted by the world, before
	 * FlecsWorldBeginPlay. The default implementation does nothing.
	 *
	 * @param InFlecsWorld World that owns this registration.
	 */
	virtual void RegisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld);

	/**
	 * Releases this object's registration from a Flecs world.
	 *
	 * Called when the object is explicitly unregistered or when its owning world
	 * is being destroyed. Implementations should release handles and other Flecs
	 * resources owned by this object. The default implementation does nothing.
	 *
	 * @param InFlecsWorld World from which this object is being unregistered.
	 */
	virtual void UnregisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld);

	/**
	 * Notifies this object that its Flecs world has entered begin play.
	 *
	 * Called after RegisterObject when the world enters begin play. Objects
	 * registered after begin play receive this callback immediately after their
	 * registration callback. Use this hook for setup that depends on the world
	 * being live, such as creating systems or observers. The default
	 * implementation does nothing.
	 *
	 * @param InFlecsWorld World that has entered begin play.
	 */
	virtual void FlecsWorldBeginPlay(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld);

	/**
	 * Requests a change to this object's registration state in a Flecs world.
	 *
	 * Reserved for future registration-state support. The current world
	 * implementation does not invoke this callback.
	 *
	 * @param InFlecsWorld World containing this object.
	 * @param InState Desired registration state.
	 */
	virtual void SetFlecsObjectState(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld, const EFlecsObjectRegistrationStateType InState);

	/**
	 * Returns this object's registration state.
	 *
	 * Registration state is not currently consumed by the world. The default
	 * implementation returns Active.
	 *
	 * @return Current registration state.
	 */
	virtual NO_DISCARD EFlecsObjectRegistrationStateType GetObjectRegistrationState() const
	{
		return EFlecsObjectRegistrationStateType::Active;
	}

	/**
	 * Determines whether this class should be included in automatic registration.
	 *
	 * Called on the class default object (CDO) during native class discovery. The
	 * implementation must be safe to call on the CDO and must not require a live
	 * Flecs world. The default implementation returns true.
	 *
	 * @return True to include this class in automatic registration.
	 */
	virtual NO_DISCARD bool ShouldAutoRegisterFromCDO() const { return true; }

	/**
	 * Determines whether this object should be registered with a specific world.
	 *
	 * Called on the newly created object before RegisterObject. Returning false
	 * prevents registration with this world. The default implementation returns
	 * true.
	 *
	 * @param InFlecsWorld Candidate world for registration.
	 * @return True to register this object with InFlecsWorld.
	 */
	virtual NO_DISCARD bool ShouldAutoRegisterWithWorld(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InFlecsWorld) const { return true; }
	
#if WITH_AUTOMATION_TESTS
	/**
	 * Determines whether this class should be automatically registered only in tests.
	 *
	 * Called on the class default object. When true, automatic discovery skips the
	 * class outside an automation test run. The default implementation returns
	 * false.
	 *
	 * @return True to limit automatic registration to automation tests.
	 */
	virtual NO_DISCARD bool ShouldAutoRegisterOnlyForTest() const { return false; }
#endif // WITH_AUTOMATION_TEST
	
	/**
	 * Returns the scope used while executing this object's registration callbacks.
	 *
	 * Unset allows the registered module or plugin defaults to determine the
	 * effective scope. None disables scoping. Custom identifier scopes require a
	 * non-empty name from GetScopeName. The default implementation returns Unset.
	 *
	 * @return Registration scope type.
	 */
	virtual NO_DISCARD EUnrealFlecsRegistrationScopeType GetRegistrationScopeType() const
	{
		return EUnrealFlecsRegistrationScopeType::Unset;
	}
	
#if WITH_EDITORONLY_DATA
	/**
	 * Determines whether this object should appear in editor registration settings.
	 *
	 * Called while building the editor settings catalog. The default
	 * implementation returns false.
	 *
	 * @return True to include this object in editor settings.
	 */
	virtual NO_DISCARD bool ShouldShowInSettings() const { return false; }
#endif // WITH_EDITORONLY_DATA
	
	/**
	 * Returns the network modes in which this object may be automatically registered.
	 *
	 * The result is a bitmask of EFlecsObjectRegistrationNetworkFlags. The default
	 * implementation enables registration for all supported network modes.
	 *
	 * @return Network registration flags.
	 */
	virtual NO_DISCARD uint8 GetObjectRegistrationNetworkFlags() const
	{
		return static_cast<uint8>(EFlecsObjectRegistrationNetworkFlags::All);
	}
	
	/**
	 * Returns the optional name used to resolve this object's registration scope.
	 *
	 * A Module scope defaults to this object's native module and a Plugin scope
	 * defaults to that module's owning Unreal plugin when this returns an empty
	 * string. Custom identifier scopes require a non-empty name.
	 *
	 * @return Explicit registration scope name, or an empty string to use the default.
	 */
	virtual NO_DISCARD FString GetScopeName() const { return ""; }

}; // class IFlecsObjectRegistrationInterface
