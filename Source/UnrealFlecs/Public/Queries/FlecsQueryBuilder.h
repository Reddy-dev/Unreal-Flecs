// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

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

USTRUCT()
struct UNREALFLECS_API FFlecsQueryBuilder
	#if CPP
		: public TFlecsQueryBuilderBase<FFlecsQueryBuilder>
	#endif // CPP
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsQueryBuilder() = default;
	
	explicit FFlecsQueryBuilder(const UFlecsWorldInterfaceObject* InWorld, const FString& InQueryName = FString());
	explicit FFlecsQueryBuilder(const UFlecsWorldInterfaceObject* InWorld, const FFlecsEntityHandle& InQueryEntity);
	
	NO_DISCARD FORCEINLINE FFlecsQueryDefinition& GetQueryDefinition_Impl() const
	{
		return const_cast<FFlecsQueryBuilder*>(this)->Definition;
	}
	
	NO_DISCARD FFlecsQuery Build() const;
	
	UPROPERTY()
	TWeakObjectPtr<const UFlecsWorldInterfaceObject> FlecsWorld;
	
	UPROPERTY()
	FString QueryName;
	
	UPROPERTY()
	TOptional<FFlecsEntityHandle> OptionalQueryEntity;
	
	UPROPERTY()
	FFlecsQueryDefinition Definition;
	
}; // struct FFlecsQueryBuilder

template <typename ...TArgs>
struct TFlecsQueryBuilder : public FFlecsQueryBuilder
{
public:
	using FFlecsQueryBuilder::FFlecsQueryBuilder;
	
	FORCEINLINE explicit TFlecsQueryBuilder(const UFlecsWorld* InWorld, const FString& InName = FString())
		: FFlecsQueryBuilder(InWorld, InName)
	{
		UE::Flecs::Queries::TAddInputTypes<TFlecsQueryBuilder, TArgs...>::Apply(*this);
	}
	
	FORCEINLINE explicit TFlecsQueryBuilder(const UFlecsWorld* InWorld, const FFlecsEntityHandle& InQueryEntity)
		: FFlecsQueryBuilder(InWorld, InQueryEntity)
	{
		UE::Flecs::Queries::TAddInputTypes<TFlecsQueryBuilder, TArgs...>::Apply(*this);
	}
	
	
}; // struct TFlecsQueryBuilder
