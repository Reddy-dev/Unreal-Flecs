// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsEntityRecord.h"
#include "Properties/FlecsComponentProperties.h"

#include "FlecsEntityRecordComponent.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsEntityRecordComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record")
	FFlecsEntityRecord EntityRecord;
	
}; // struct FFlecsEntityRecordComponent

template <>
struct TFlecsComponentTraits<FFlecsEntityRecordComponent> : public TFlecsComponentTraitsBase<FFlecsEntityRecordComponent>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::DontInherit;
}; // struct TFlecsComponentTraits<FFlecsEntityRecordComponent>

