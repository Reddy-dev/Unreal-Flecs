// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once


#include "FlecsQueryBuilderBase.h"
#include "FlecsQuery.h"

#include "FlecsQueryBuilder.generated.h"

class UFlecsWorldInterfaceObject;

namespace UE::Flecs::Queries
{
	template <typename TBuilder, typename T>
	struct TAddInputType
	{
		static FORCEINLINE void Apply(TBuilder& InOutDefinition)
		{
			InOutDefinition
				.template With<T>().InOutExpression(TypeToInOut<T>()).Oper(TypeToOperator<T>());
		}
		
	}; // struct TAddInputType

	template <typename TBuilder, typename ...TArgs>
	struct TAddInputTypes
	{
		static FORCEINLINE void Apply(TBuilder InOutDefinition)
		{
			// Base case: do nothing
		}
		
	}; // struct TAddInputTypes
	
	template <typename TBuilder, typename TFirst, typename ...TRest>
	struct TAddInputTypes<TBuilder, TFirst, TRest...>
	{
		static FORCEINLINE void Apply(TBuilder& InOutDefinition)
		{
			UE::Flecs::Queries::TAddInputType<TBuilder, TFirst>::Apply(InOutDefinition);
			TAddInputTypes<TBuilder, TRest...>::Apply(InOutDefinition);
		}
		
	}; // struct TAddInputTypes
	
} // namespace UE::Flecs::Queries

/**
 * @brief Fluent builder for an Unreal-Flecs query definition.
 *
 * The builder records the query in an Unreal-owned
 * FFlecsQueryDefinition. Call Build() to resolve that definition against the
 * associated world and create the native-backed FFlecsQuery. Query terms and
 * query options are configured through TFlecsQueryBuilderBase.
 *
 * Use the entity constructor when updating an existing query entity. The
 * resulting builder still uses the same fluent configuration API, but Build()
 * applies the definition to that entity instead of creating a new named query.
 *
 * @see TFlecsQueryBuilderBase
 * @see https://www.flecs.dev/flecs/Queries.html
 */
USTRUCT()
struct UNREALFLECS_API FFlecsQueryBuilder
	#if CPP
		: public TFlecsQueryBuilderBase<FFlecsQueryBuilder>
	#endif // CPP
{
	GENERATED_BODY()
	
public:
	/** Creates an empty builder without an associated world. */
	FORCEINLINE FFlecsQueryBuilder() = default;
	
	/**
	 * @brief Creates a builder that will create a query in a world.
	 *
	 * @param InWorld World that owns the query.
	 * @param InQueryName Optional name for the query entity.
	 */
	explicit FFlecsQueryBuilder(const UFlecsWorldInterfaceObject* InWorld, const FString& InQueryName = FString());
	
	/**
	 * @brief Creates a builder that will configure an existing query entity.
	 *
	 * @param InWorld World that owns the query entity.
	 * @param InQueryEntity Existing query entity to configure.
	 */
	explicit FFlecsQueryBuilder(const UFlecsWorldInterfaceObject* InWorld, const FFlecsEntityHandle& InQueryEntity);
	
	/**
	 * @brief Returns the mutable definition used by the fluent base.
	 *
	 * This accessor is the CRTP customization point used by
	 * TFlecsQueryBuilderBase and is not a native Flecs query handle.
	 */
	NO_DISCARD FORCEINLINE FFlecsQueryDefinition& GetQueryDefinition_Impl() const
	{
		return const_cast<FFlecsQueryBuilder*>(this)->Definition;
	}
	
	/**
	 * @brief Builds the configured query in the associated world.
	 *
	 * @return A native-backed query handle.
	 * @warning The builder must have a valid world before Build() is called.
	 */
	NO_DISCARD FFlecsQuery Build() const;
	
	/**
	 * @brief Builds the configured query with compile-time iterator fields.
	 *
	 * @tparam TArgs Component field types exposed by the resulting iterable.
	 * @return A typed query whose iterator fields correspond to TArgs.
	 */
	template <typename ...TArgs>
	NO_DISCARD TTypedFlecsQuery<TArgs...> BuildTyped() const
	{
		return TTypedFlecsQuery<TArgs...>(Build());
	}
	
	/** World used to resolve the query definition. */
	UPROPERTY()
	TWeakObjectPtr<const UFlecsWorldInterfaceObject> FlecsWorld;
	
	/** Optional name used when creating a new query entity. */
	UPROPERTY()
	FString QueryName;
	
	/** Existing query entity to update, when the entity-based constructor is used. */
	UPROPERTY()
	TOptional<FFlecsEntityHandle> OptionalQueryEntity;
	
	/** Unreal-owned query definition accumulated by the fluent API. */
	UPROPERTY()
	FFlecsQueryDefinition Definition;
	
}; // struct FFlecsQueryBuilder

/**
 * @brief Typed query builder that adds terms for TArgs at construction time.
 *
 * Each component argument is translated into a query term and its default
 * access mode. Pointer arguments are optional terms; const and reference
 * arguments select the corresponding read/write access mode.
 *
 * @tparam TArgs Component or field argument types used to seed the query.
 */
template <typename ...TArgs>
struct TFlecsQueryBuilder : public FFlecsQueryBuilder
{
public:
	using FFlecsQueryBuilder::FFlecsQueryBuilder;
	
	/**
	 * @brief Creates a typed builder for a new named query.
	 *
	 * @param InWorld World that owns the query.
	 * @param InName Optional name for the query entity.
	 */
	FORCEINLINE explicit TFlecsQueryBuilder(const UFlecsWorldInterfaceObject* InWorld, const FString& InName = FString())
		: FFlecsQueryBuilder(InWorld, InName)
	{
		UE::Flecs::Queries::TAddInputTypes<TFlecsQueryBuilder, TArgs...>::Apply(*this);
	}
	
	/**
	 * @brief Creates a typed builder for an existing query entity.
	 *
	 * @param InWorld World that owns the query entity.
	 * @param InQueryEntity Existing query entity to configure.
	 */
	FORCEINLINE explicit TFlecsQueryBuilder(const UFlecsWorldInterfaceObject* InWorld, const FFlecsEntityHandle& InQueryEntity)
		: FFlecsQueryBuilder(InWorld, InQueryEntity)
	{
		UE::Flecs::Queries::TAddInputTypes<TFlecsQueryBuilder, TArgs...>::Apply(*this);
	}
	
	
}; // struct TFlecsQueryBuilder
