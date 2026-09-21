// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "FlecsEntityRecord.h"
#include "Properties/FlecsComponentProperties.h"

#include "FlecsEntityRecordComponent.generated.h"

/**
 * @brief Stores an entity record as a non-inherited Flecs component.
 *
 * This component preserves the record definition on an entity such as a
 * Collection prefab for inspection and later materialization.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsEntityRecordComponent
{
	GENERATED_BODY()
	
	/** Keeps this component on the owning prefab instead of inheriting it. */
	static constexpr flecs::on_instantiate OnInstantiate = flecs::on_instantiate::dont_inherit;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record")
	/** Entity record stored by the component. */
	FFlecsEntityRecord EntityRecord;
	
}; // struct FFlecsEntityRecordComponent

template <>
struct TFlecsComponentTraits<FFlecsEntityRecordComponent> : public TFlecsComponentTraitsBase<FFlecsEntityRecordComponent>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::DontInherit;
}; // struct TFlecsComponentTraits<FFlecsEntityRecordComponent>

