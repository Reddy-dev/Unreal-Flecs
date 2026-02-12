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
	using TNoCVRef = std::remove_cv_t<std::remove_reference_t<T>>;

	template <typename T>
	inline constexpr bool bIsPointerV = std::is_pointer_v<TNoCVRef<T>>;

	template <typename T>
	using TComponentFromArg = std::remove_pointer_t<TNoCVRef<T>>;

	template <typename T>
	using TComponentBare = std::remove_cv_t<TComponentFromArg<T>>;

	template <typename T>
	inline constexpr bool bIsConstPointeeOrValueV =
		std::is_const_v<std::remove_reference_t<T>> || // const T / const T&
		std::is_const_v<std::remove_pointer_t<TNoCVRef<T>>>; // const T*

	template <typename T>
	inline constexpr bool bIsRefV = std::is_reference_v<T>;
	
	template <typename T>
	FORCEINLINE constexpr EFlecsQueryInOut TypeToInOut()
	{
		if constexpr (bIsConstPointeeOrValueV<T>)
		{
			return EFlecsQueryInOut::Read;
		}
		else if constexpr (bIsRefV<T>)
		{
			return EFlecsQueryInOut::ReadWrite;
		}
		else
		{
			return EFlecsQueryInOut::Default;
		}
	}
	
	template <typename T>
	FORCEINLINE constexpr EFlecsQueryOperator TypeToOperator()
	{
		if constexpr (bIsPointerV<T>)
		{
			return EFlecsQueryOperator::Optional;
		}
		else
		{
			return EFlecsQueryOperator::And;
		}
	}
	
	template <typename T>
	struct TAddInputType
	{
		static FORCEINLINE void Apply(TFlecsQueryBuilderBase<FFlecsQueryBuilder>& InOutDefinition)
		{
			InOutDefinition
				.With<T>().InOutExpression(TypeToInOut<T>()).Oper(TypeToOperator<T>());
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
	explicit FFlecsQueryBuilder(const UFlecsWorld* InWorld, const FFlecsEntityHandle& InQueryEntity);
	
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
		Unreal::Flecs::Queries::TAddInputTypes<TArgs...>::Apply(*this);
	}
	
	FORCEINLINE explicit TFlecsQueryBuilder(const UFlecsWorld* InWorld, const FFlecsEntityHandle& InQueryEntity)
		: FFlecsQueryBuilder(InWorld, InQueryEntity)
	{
		Unreal::Flecs::Queries::TAddInputTypes<TArgs...>::Apply(*this);
	}
	
	
}; // struct TFlecsQueryBuilder
