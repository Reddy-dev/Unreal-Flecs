// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsQueryBuilderBase.h"
#include "FlecsQuery.h"

#include "FlecsQueryBuilder.generated.h"

class UFlecsWorld;

namespace Unreal::Flecs::Queries
{
	template <typename T>
	struct TAddInputType
	{
		static FORCEINLINE void Apply(TFlecsQueryBuilderBase<FFlecsQueryBuilder>& InOutDefinition)
		{
			InOutDefinition.With<T>();
		}
		
	}; // struct TAddInputType

	template <typename ...TArgs>
	struct TAddInputTypes
	{
		static FORCEINLINE void Apply(TFlecsQueryBuilderBase<FFlecsQueryBuilder>& InOutDefinition)
		{
			// Base case: do nothing
		}
		
	}; // struct TAddInputTypes
	
	template <typename TFirst, typename ...TRest>
	struct TAddInputTypes<TFirst, TRest...>
	{
		static FORCEINLINE void Apply(TFlecsQueryBuilderBase<FFlecsQueryBuilder>& InOutDefinition)
		{
			Unreal::Flecs::Queries::TAddInputType<TFirst>::Apply(InOutDefinition);
			TAddInputTypes<TRest...>::Apply(InOutDefinition);
		}
		
	}; // struct TAddInputTypes
	
} // namespace Unreal::Flecs::Queries

USTRUCT()
struct UNREALFLECS_API FFlecsQueryBuilder 
#if CPP
	: public TFlecsQueryBuilderBase<FFlecsQueryBuilder>
#endif // CPP
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsQueryBuilder() = default;
	
	explicit FFlecsQueryBuilder(const UFlecsWorld* InWorld, const FString& InName = FString());
	
	NO_DISCARD FORCEINLINE FFlecsQueryDefinition& GetQueryDefinition_Impl() const
	{
		return const_cast<FFlecsQueryBuilder*>(this)->Definition;
	}
	
	NO_DISCARD FFlecsQuery Build() const;
	
	UPROPERTY()
	TWeakObjectPtr<const UFlecsWorld> World;
	
	UPROPERTY()
	FString Name;
	
	UPROPERTY()
	FFlecsQueryDefinition Definition;
	
}; // struct FFlecsQueryBuilder

template <typename ...TArgs>
struct TFlecsQueryBuilder : public FFlecsQueryBuilder
{
public:
	using FFlecsQueryBuilder::FFlecsQueryBuilder;
	
	FORCEINLINE TFlecsQueryBuilder(const UFlecsWorld* InWorld, const FString& InName = FString())
		: FFlecsQueryBuilder(InWorld, InName)
	{
		Unreal::Flecs::Queries::TAddInputTypes<TArgs...>::Apply(*this);
	}
	
	
}; // struct TFlecsQueryBuilder
