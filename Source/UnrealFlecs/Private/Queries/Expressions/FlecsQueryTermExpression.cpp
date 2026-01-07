// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/Expressions/FlecsQueryTermExpression.h"

#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryTermExpression)

FFlecsQueryTermExpression::FFlecsQueryTermExpression() : Super(true /* bInAllowsChildExpressions */)
{
}

void FFlecsQueryTermExpression::Apply(TSolidNotNull<const UFlecsWorld*> InWorld, flecs::query_builder<>& InQueryBuilder) const
{
	switch (InputType.Type)
	{
		case EFlecsQueryInputType::ScriptStruct:
			{
				const UScriptStruct* Struct = InputType.ScriptStruct;
				solid_checkf(IsValid(Struct),
					TEXT("Invalid ScriptStruct provided for query term expression"));
				
				const FFlecsEntityHandle ScriptStructEntity = InWorld->GetScriptStructEntity(Struct);
				solid_check(ScriptStructEntity.IsValid());
				
				InQueryBuilder.with(ScriptStructEntity);

				break;
			}
		case EFlecsQueryInputType::Entity:
			{
				const FFlecsId Entity = InputType.Entity;
				solid_checkf(Entity.IsValid(),
					TEXT("Invalid Entity provided for query term expression"));
				InQueryBuilder.with(Entity);

				break;
			}
		case EFlecsQueryInputType::String:
			{
				const FString Expr = InputType.Expr.Expr;
				solid_checkf(!Expr.IsEmpty(),
					TEXT("Empty string provided for query term expression"));
				
				InQueryBuilder.with(StringCast<char>(*Expr).Get());
				
				break;
			}
		case EFlecsQueryInputType::GameplayTag:
			{
				const FGameplayTag Tag = InputType.Tag;
				solid_checkf(Tag.IsValid(),
					TEXT("Invalid GameplayTag provided for query term expression"));
				
				const FFlecsEntityHandle TagEntity = InWorld->GetTagEntity(Tag);
				solid_check(TagEntity.IsValid());
				
				InQueryBuilder.with(TagEntity);

				break;
			}
		case EFlecsQueryInputType::CPPType:
			{
				const FString StringType = InputType.CPPType.ToString();
				solid_checkf(!StringType.IsEmpty(),
					TEXT("Empty string provided for query term expression"));
				
				const FFlecsEntityHandle TypeEntity = InWorld->LookupEntityBySymbol_Internal(StringType);
				solid_checkf(TypeEntity.IsValid(),
					TEXT("Could not find entity for CPP Type '%s'"), *StringType);
				
				InQueryBuilder.with(TypeEntity);
				
				break;
			}
	}

	if (bWithout)
	{
		InQueryBuilder.not_();
	}

	Super::Apply(InWorld, InQueryBuilder);
}