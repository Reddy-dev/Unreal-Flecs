// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FlecsCollectionInterface.generated.h"

struct FFlecsCollectionBuilder;

// This class does not need to be modified.
UINTERFACE(meta = (CannotImplementInterfaceInBlueprint))
class UFlecsCollectionInterface : public UInterface
{
	GENERATED_BODY()
}; // class UFlecsCollectionInterface

class UNREALFLECS_API IFlecsCollectionInterface
{
	GENERATED_BODY()

public:
	virtual void BuildCollection(FFlecsCollectionBuilder& Builder) const PURE_VIRTUAL(IFlecsCollectionInterface::BuildCollection, );
	

}; // class IFlecsCollectionInterface
