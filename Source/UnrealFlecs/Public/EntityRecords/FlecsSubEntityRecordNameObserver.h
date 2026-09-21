// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "Observers/FlecsObserverObject.h"

#include "FlecsSubEntityRecordNameObserver.generated.h"

UCLASS(NotBlueprintable, BlueprintType)
/**
 * @brief Observes named sub-entities and assigns their Flecs names.
 *
 * The observer reacts when a sub-entity name component is set on a child of an
 * entity record hierarchy.
 */
class UNREALFLECS_API UFlecsSubEntityRecordNameObserver : public UFlecsObserverObject
{
	GENERATED_BODY()

public:
	/**
	 * @brief Constructs the sub-entity name observer.
	 * @param ObjectInitializer Unreal object initializer.
	 */
	UFlecsSubEntityRecordNameObserver(const FObjectInitializer& ObjectInitializer);
	
	/**
	 * @brief Configures the Flecs query and observer events.
	 * @param InWorld World receiving the observer.
	 * @param InOutBuilder Observer builder to configure.
	 */
	virtual void BuildObserver(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, TFlecsObserverBuilder<>& InOutBuilder) const override;
	/**
	 * @brief Assigns each observed sub-entity its configured name.
	 * @param InWorld World containing the observed entity.
	 * @param InIterator Current observer iterator.
	 * @param InIndex Index of the current iterator entity.
	 */
	virtual void EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, flecs::iter& InIterator, const FFlecsId InIndex) override;
	
}; // class UFlecsSubEntityRecordNameObserver
