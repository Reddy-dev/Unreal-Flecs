// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once


#include "Properties/FlecsComponentProperties.h"

#include "FlecsDirtyObserverTag.generated.h"

// All component dirty observers have this tag (they are created in UFlecsNetworkWorldSubsystem::RegisterIndividualComponentDirtyObserver)
USTRUCT()
struct FFlecsDirtyObserverTag
{
	GENERATED_BODY()
}; // struct FFlecsDirtyObserverTag

// @TODO: DontFragment?
template <>
struct TFlecsComponentTraits<FFlecsDirtyObserverTag> : public TFlecsComponentTraitsBase<FFlecsDirtyObserverTag>
{
}; // struct TFlecsComponentTraits<FFlecsDirtyObserverTag>
