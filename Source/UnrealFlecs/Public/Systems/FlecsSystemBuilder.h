// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once


#include "FlecsSystemBuilderBase.h"
#include "FlecsSystemHandle.h"
#include "Queries/FlecsQueryBuilder.h"

/**
 * @brief Fluent builder for an Unreal-Flecs system.
 *
 * A system builder combines the query configuration inherited from
 * TFlecsQueryBuilderBase with scheduling, phase, tick, and callback options.
 * Calling run(), each(), or run_each() stores the callback and materializes
 * the system as an FFlecsSystemHandle.
 *
 * @tparam TComponents Component field types passed to each() and run_each().
 * @see TFlecsSystemBuilderBase
 * @see https://www.flecs.dev/flecs/Systems.html
 */
template <typename ...TComponents>
struct TFlecsSystemBuilder : public TFlecsSystemBuilderBase<TFlecsSystemBuilder<TComponents...>, FFlecsSystemHandle, TComponents...>
{
	friend struct TFlecsSystemBuilderBase<TFlecsSystemBuilder, FFlecsSystemHandle, TComponents...>;
	
public:
	/**
	 * @brief Returns the mutable system definition used by the fluent base.
	 *
	 * This is the CRTP customization point that exposes the system definition
	 * and its embedded query definition.
	 */
	FORCEINLINE FFlecsSystemDefinition& GetSystemDefinition_Impl() const
	{
		return const_cast<FFlecsSystemDefinition&>(SystemDefinition);
	}
	
	/**
	 * @brief Creates a system builder for a world.
	 *
	 * @param InWorld World that owns the system.
	 * @param InOptionalName Optional name for the system entity.
	 * @param InSystemDefinition Initial definition to copy into the builder.
	 */
	FORCEINLINE TFlecsSystemBuilder(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, const FString& InOptionalName, 
		const FFlecsSystemDefinition& InObserverDefinition = FFlecsSystemDefinition())
									: SystemDefinition(InObserverDefinition)
									, FlecsWorld(InWorld)
									, OptionalName(InOptionalName)
	{
		UE::Flecs::Queries::TAddInputTypes<TFlecsSystemBuilder, TComponents...>::Apply(*this);
	}
	
	/** Unreal-owned system definition accumulated by the fluent API. */
	FFlecsSystemDefinition SystemDefinition;
	
	/** World used to resolve and create the system. */
	TWeakObjectPtr<const UFlecsWorldInterfaceObject> FlecsWorld;
	
	/** Optional name used when creating the system entity. */
	FString OptionalName;
	
protected:
	
	/** Materializes the configured system and returns its handle. */
	FORCEINLINE FFlecsSystemHandle CreateSystem() const
	{
		solid_checkf(FlecsWorld.IsValid(), TEXT("World is not valid."));
		
		return FFlecsSystemHandle(FlecsWorld.Get(), SystemDefinition, OptionalName);
	}
	
	/** Materializes a system configured with a run callback. */
	FORCEINLINE FFlecsSystemHandle CreateRunSystem() const
	{
		return CreateSystem();
	}
	
	/** Materializes a system configured with a run-each callback. */
	FORCEINLINE FFlecsSystemHandle CreateRunEachSystem() const
	{
		return CreateSystem();
	}
	
	/** Materializes a system configured with an each callback. */
	FORCEINLINE FFlecsSystemHandle CreateEachSystem() const
	{
		return CreateSystem();
	}
	
}; // struct TFlecsSystemBuilder
