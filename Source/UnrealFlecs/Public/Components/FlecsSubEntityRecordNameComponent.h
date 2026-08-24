// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "Properties/FlecsComponentProperties.h"

#include "FlecsSubEntityRecordNameComponent.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsSubEntityRecordNameComponent
{
	GENERATED_BODY()
	
	static constexpr bool DontFragment = true;
	
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
