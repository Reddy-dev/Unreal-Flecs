// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "FlecsObserverBuilderBase.h"
#include "FlecsObserverHandle.h"
#include "Queries/FlecsQueryBuilder.h"

class UFlecsWorldInterfaceObject;

/**
 * @brief Fluent builder for an Unreal-Flecs observer.
 *
 * An observer builder combines query-term configuration with event filters,
 * observer flags, and callback options. Calling run(), each(), or run_each()
 * stores the callback and materializes the observer as an
 * FFlecsObserverHandle.
 *
 * @tparam TComponents Component field types passed to each() and run_each().
 * @see TFlecsObserverBuilderBase
 * @see https://www.flecs.dev/flecs/ObserversManual.html
 */
template <typename ...TComponents>
struct TFlecsObserverBuilder : public TFlecsObserverBuilderBase<TFlecsObserverBuilder<TComponents...>, FFlecsObserverHandle, TComponents...>
{
	friend struct TFlecsObserverBuilderBase<TFlecsObserverBuilder<TComponents...>, FFlecsObserverHandle, TComponents...>;
	
public:
	/**
	 * @brief Returns the mutable observer definition used by the fluent base.
	 *
	 * This is the CRTP customization point that exposes the observer
	 * definition and its embedded query definition.
	 */
	FORCEINLINE FFlecsObserverDefinition& GetObserverDefinition_Impl() const
	{
		return const_cast<FFlecsObserverDefinition&>(ObserverDefinition);
	}
	
	/**
	 * @brief Creates an observer builder for a world.
	 *
	 * @param InWorld World that owns the observer.
	 * @param InOptionalName Optional name for the observer entity.
	 * @param InObserverDefinition Initial definition to copy into the builder.
	 */
	FORCEINLINE TFlecsObserverBuilder(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, const FString& InOptionalName, 
		const FFlecsObserverDefinition& InObserverDefinition = FFlecsObserverDefinition())
									: ObserverDefinition(InObserverDefinition)
									, FlecsWorld(InWorld)
									, OptionalName(InOptionalName)
	{
		UE::Flecs::Queries::TAddInputTypes<TFlecsObserverBuilder, TComponents...>::Apply(*this);
	}

	/** Unreal-owned observer definition accumulated by the fluent API. */
	FFlecsObserverDefinition ObserverDefinition;
	
	/** World used to resolve and create the observer. */
	TWeakObjectPtr<const UFlecsWorldInterfaceObject> FlecsWorld;
	
	/** Optional name used when creating the observer entity. */
	FString OptionalName;
	
protected:
	
	/** Materializes the configured observer and returns its handle. */
	FORCEINLINE FFlecsObserverHandle CreateObserver() const
	{
		solid_checkf(FlecsWorld.IsValid(), TEXT("World is not valid."));
		
		return FFlecsObserverHandle(FlecsWorld.Get(), ObserverDefinition, OptionalName);
	}
	
	/** Materializes an observer configured with a run callback. */
	FORCEINLINE FFlecsObserverHandle CreateRunObserver() const
	{
		return CreateObserver();
	}
	
	/** Materializes an observer configured with a run-each callback. */
	FORCEINLINE FFlecsObserverHandle CreateRunEachObserver() const
	{
		return CreateObserver();
	}
	
	/** Materializes an observer configured with an each callback. */
	FORCEINLINE FFlecsObserverHandle CreateEachObserver() const
	{
		return CreateObserver();
	}
	
}; // struct TFlecsObserverBuilder
