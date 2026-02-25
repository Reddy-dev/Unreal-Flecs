// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "Entities/FlecsId.h"

#include "FlecsIteratorObjectInterface.generated.h"

class UFlecsWorld;

// This class does not need to be modified.
UINTERFACE(meta = (CannotImplementInterfaceInBlueprint))
class UFlecsIteratorObjectInterface : public UInterface
{
	GENERATED_BODY()
}; // class UFlecsIteratorObjectInterface

/**
 * 
 */
class UNREALFLECS_API IFlecsIteratorObjectInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void RunIterator(const TSolidNotNull<const UFlecsWorld*> InWorld, flecs::iter& InIterator);
	virtual void EachIterator(const TSolidNotNull<const UFlecsWorld*> InWorld, flecs::iter& InIterator, const FFlecsId InIndex);

}; // class IFlecsIteratorObjectInterface
