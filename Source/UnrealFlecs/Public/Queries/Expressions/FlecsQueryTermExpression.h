// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsOperQueryExpression.h"
#include "FlecsQueryExpression.h"
#include "Queries/FlecsQueryInputType.h"

#include "FlecsQueryTermExpression.generated.h"

USTRUCT(BlueprintType, meta = (DisplayName = "Term Query"))
struct UNREALFLECS_API FFlecsQueryTermExpression : public FFlecsQueryExpression
{
	GENERATED_BODY()

public:
	FFlecsQueryTermExpression();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	bool bIsPair = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	FFlecsQueryInput InputType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query", meta = (EditCondition = "bIsPair", EditConditionHides))
	FFlecsQueryInput SecondInputType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	bool bWithout = false;
	
	template <Unreal::Flecs::Queries::CQueryInputType TInput>
	FORCEINLINE FFlecsQueryTermExpression& SetInput(const TInput& InInput)
	{
		if constexpr (std::is_convertible<TInput, const UScriptStruct*>::value)
		{
			InputType.Type = EFlecsQueryInputType::ScriptStruct;
			InputType.ScriptStruct = InInput;
		}
		else if constexpr (std::is_convertible<TInput, FFlecsId>::value)
		{
			InputType.Type = EFlecsQueryInputType::Entity;
			InputType.Entity = InInput;
		}
		else if constexpr (std::is_convertible<TInput, FString>::value)
		{
			InputType.Type = EFlecsQueryInputType::String;
			InputType.Expr.Expr = InInput;
		}
		else if constexpr (std::is_convertible<TInput, FGameplayTag>::value)
		{
			InputType.Type = EFlecsQueryInputType::GameplayTag;
			InputType.Tag = InInput;
		}
		else
		{
			static_assert(false, "Type TInput is not a valid Flecs Entity Function Input Type");
			UNREACHABLE;
		}

		return *this;
	}
	
	template <Unreal::Flecs::Queries::CQueryInputType TInput>
	FORCEINLINE FFlecsQueryTermExpression& SetSecondInput(const TInput& InInput)
	{
		if constexpr (std::is_convertible<TInput, const UScriptStruct*>::value)
		{
			SecondInputType.Type = EFlecsQueryInputType::ScriptStruct;
			SecondInputType.ScriptStruct = InInput;
		}
		else if constexpr (std::is_convertible<TInput, FFlecsId>::value)
		{
			SecondInputType.Type = EFlecsQueryInputType::Entity;
			SecondInputType.Entity = InInput;
		}
		else if constexpr (std::is_convertible<TInput, FString>::value)
		{
			SecondInputType.Type = EFlecsQueryInputType::String;
			SecondInputType.Expr.Expr = InInput;
		}
		else if constexpr (std::is_convertible<TInput, FGameplayTag>::value)
		{
			SecondInputType.Type = EFlecsQueryInputType::GameplayTag;
			SecondInputType.Tag = InInput;
		}
		else
		{
			static_assert(false, "Type TInput is not a valid Flecs Entity Function Input Type");
			UNREACHABLE;
		}
		
		bIsPair = true;
		
		return *this;
	}
	
	template <Solid::TScriptStructConcept TInput>
	FORCEINLINE FFlecsQueryTermExpression& SetInput()
	{
		InputType.Type = EFlecsQueryInputType::ScriptStruct;
		InputType.ScriptStruct = TBaseStructure<TInput>::Get();
		return *this;
	}
	
	template <Solid::TScriptStructConcept TInput>
	FORCEINLINE FFlecsQueryTermExpression& SetSecondInput()
	{
		SecondInputType.Type = EFlecsQueryInputType::ScriptStruct;
		SecondInputType.ScriptStruct = TBaseStructure<TInput>::Get();
		bIsPair = true;
		return *this;
	}
	
	// CPP Type input
	template <typename TInput>
	requires (!Unreal::Flecs::Queries::CQueryInputType<TInput> && !Solid::TScriptStructConcept<TInput>)
	FORCEINLINE FFlecsQueryTermExpression& SetInput()
	{
		InputType.Type = EFlecsQueryInputType::CPPType;
		const std::string_view TypeName = nameof(TInput);
		InputType.CPPType = FName(TypeName.data());
		return *this;
	}
	
	template <typename TInput>
	requires (!Unreal::Flecs::Queries::CQueryInputType<TInput> && !Solid::TScriptStructConcept<TInput>)
	FORCEINLINE FFlecsQueryTermExpression& SetSecondInput()
	{
		SecondInputType.Type = EFlecsQueryInputType::CPPType;
		const std::string_view TypeName = nameof(TInput);
		SecondInputType.CPPType = FName(TypeName.data());
		bIsPair = true;
		return *this;
	}
	
	/*template <typename TInput>
	requires (std::is_enum!Unreal::Flecs::Queries::CQueryInputType<TInput> && !Solid::TStaticEnumConcept<TInput> 
	FORCEINLINE FFlecsQueryTermExpression*/

	virtual void Apply(const TSolidNotNull<const UFlecsWorld*> InWorld, flecs::query_builder<>& InQueryBuilder) const override;
	
}; // struct FFlecsQueryTermExpression
