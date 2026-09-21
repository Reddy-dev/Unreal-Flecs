// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "Properties/FlecsComponentProperties.h"

#include "FlecsSubEntityRecordNameComponent.generated.h"

USTRUCT(BlueprintType)
/**
 * @brief Stores the name propagated to a materialized sub-entity.
 *
 * The record-name observer consumes this component and sets the entity's
 * Flecs identifier name.
 */
struct UNREALFLECS_API FFlecsSubEntityRecordNameComponent
{
	GENERATED_BODY()
	
	/** Keeps the name component from being fragmented across table storage. */
	static constexpr bool DontFragment = true;
	
public:

	UPROPERTY(EditAnywhere, Category = "Flecs")
	/** Name assigned to the sub-entity by the record-name observer. */
	FString SubEntityName;
	
}; // struct FFlecsSubEntityRecordNameComponent

template <>
struct TFlecsComponentTraits<FFlecsSubEntityRecordNameComponent> : public TFlecsComponentTraitsBase<FFlecsSubEntityRecordNameComponent>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Override;
	
	static constexpr bool DontFragment = true;
	
	static constexpr bool UseLowId = false;
	
}; // struct TFlecsComponentTraits<FFlecsSubEntityRecordNameComponent>
