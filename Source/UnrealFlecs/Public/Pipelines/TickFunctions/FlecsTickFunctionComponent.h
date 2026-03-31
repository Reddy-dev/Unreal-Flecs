// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/EngineBaseTypes.h"

#include "Properties/FlecsComponentProperties.h"
#include "FlecsTickFunction.h"

#include "FlecsTickFunctionComponent.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsTickFunctionComponent
{
	GENERATED_BODY()

public:
	//UPROPERTY() // @TODO: this is disabled due to a UHT bug
	TSharedStruct<FFlecsTickFunction> TickFunction;
	
}; // struct FFlecsTickFunctionComponent

template <>
struct TFlecsComponentTraits<FFlecsTickFunctionComponent> : public TFlecsComponentTraitsBase<FFlecsTickFunctionComponent>
{ 	
	static constexpr bool Sparse = true;
}; // struct TFlecsComponentTraits<FFlecsTickFunctionComponent>
