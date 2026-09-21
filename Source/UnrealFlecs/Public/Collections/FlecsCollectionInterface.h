// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"
#include "StructUtils/InstancedStruct.h"

#include "FlecsCollectionInterface.generated.h"

struct FFlecsEntityHandle;
struct FFlecsCollectionBuilder;

// This class does not need to be modified.
/**
 * @brief Unreal reflection wrapper for the Collection interface.
 *
 * Collection classes implement IFlecsCollectionInterface in native C++ and
 * describe their prefab composition through BuildCollection().
 */
UINTERFACE(meta = (CannotImplementInterfaceInBlueprint))
class UFlecsCollectionInterface : public UInterface
{
	GENERATED_BODY()
}; // class UFlecsCollectionInterface

class UNREALFLECS_API IFlecsCollectionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * @brief Builds this class's Collection definition.
	 * @param Builder Builder used to add components, child entities,
	 * references, and parameter behavior.
	 */
	virtual void BuildCollection(FFlecsCollectionBuilder& Builder) const = 0;

	// @TODO: Unused
	/**
	 * @brief Invokes the optional parameter-instantiation hook when configured.
	 * @param InEntityHandle Entity receiving the Collection instance.
	 * @param InParameters Parameters supplied to the Collection.
	 */
	void CallInstantiateParameters(const FFlecsEntityHandle& InEntityHandle, const FInstancedStruct& InParameters) const;

	// @TODO: Currently unused
	/**
	 * @brief Applies Collection parameters through an overrideable hook.
	 * @param InEntityHandle Entity receiving the Collection instance.
	 * @param InParameters Parameters supplied to the Collection.
	 *
	 * The default implementation does nothing.
	 */
	virtual void InstantiateParameters(const FFlecsEntityHandle& InEntityHandle, const FInstancedStruct& InParameters) const {}

	// @TODO: Currently unused
	// Defaults to an invalid FInstancedStruct (this is optional)
	/**
	 * @brief Returns the parameter type and default value for this Collection.
	 * @return An FInstancedStruct describing the parameters, or an invalid
	 * struct when this optional hook is unused.
	 */
	virtual FInstancedStruct GetParametersType() const;
	
/**
 * @brief Interface implemented by UObject classes that define a Collection.
 *
 * A class implementing this interface can be registered with
 * UFlecsCollectionWorldSubsystem::RegisterCollectionInterfaceClass(). Its
 * class default object is asked to build the Collection definition.
 */
}; // class IFlecsCollectionInterface
