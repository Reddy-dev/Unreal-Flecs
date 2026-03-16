// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsSubEntityRecordNameComponent.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsSubEntityRecordNameComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Flecs")
	FString SubEntityName;
	
}; // struct FFlecsSubEntityRecordNameComponent

template <>
struct TFlecsComponentTraits<FFlecsSubEntityRecordNameComponent> : public TFlecsComponentTraitsBase<FFlecsSubEntityRecordNameComponent>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Override;
	
	static constexpr bool DontFragment = true;
	
}; // struct TFlecsComponentTraits<FFlecsSubEntityRecordNameComponent>