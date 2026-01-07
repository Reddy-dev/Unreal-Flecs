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
	FFlecsQueryInput InputType;

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
	
	template <Solid::TScriptStructConcept TInput>
	FORCEINLINE FFlecsQueryTermExpression& SetInput()
	{
		InputType.Type = EFlecsQueryInputType::ScriptStruct;
		InputType.ScriptStruct = TBaseStructure<TInput>::Get();
		return *this;
	}
	
	// CPP Type input
	template <typename TInput>
	requires (!Unreal::Flecs::Queries::CQueryInputType<TInput>)
	FORCEINLINE FFlecsQueryTermExpression& SetInput()
	{
		InputType.Type = EFlecsQueryInputType::String;
		const std::string_view TypeName = nameof(TInput);
		InputType.Expr.Expr = StringCast<TCHAR>(TypeName.data()).Get();
		return *this;
	}

	virtual void Apply(const TSolidNotNull<const UFlecsWorld*> InWorld, flecs::query_builder<>& InQueryBuilder) const override;
	
}; // struct FFlecsQueryTermExpression
