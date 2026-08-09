// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Properties/FlecsComponentProperties.h"

#include "FlecsViewerType.generated.h"

UENUM(BlueprintType)
enum class EFlecsViewerType : uint8
{
	Player,
	Actor,
	StreamingSource
}; // enum class EFlecsViewerType

template <>
struct TFlecsComponentTraits<EFlecsViewerType> : public TFlecsComponentTraitsBase<EFlecsViewerType>
{
	static constexpr bool Exclusive = true;
}; // struct TFlecsComponentTraits<EFlecsViewerType>


