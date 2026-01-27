// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FlecsQueryFlags.h"
#include "Enums/FlecsQueryCache.h"
#include "Expressions/FlecsQueryTermExpression.h"
#include "FlecsQueryDefinition.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsQueryDefinition
{
	GENERATED_BODY()

public:
	FORCEINLINE FFlecsQueryDefinition() = default;

	FORCEINLINE FFlecsQueryDefinition& AddQueryTerm(const FFlecsQueryTermExpression& InTerm)
	{
		Terms.Add(InTerm);
		return *this;
	}

	template <Unreal::Flecs::Queries::TQueryExpressionConcept TExpression>
	FORCEINLINE FFlecsQueryDefinition& AddAdditionalExpression(const TExpression& InExpression)
	{
		OtherExpressions.Add(InExpression);
		return *this;
	}
	
	NO_DISCARD FORCEINLINE int32 GetNumTerms() const
	{
		return Terms.Num();
	}
	
	NO_DISCARD FORCEINLINE bool IsValidTermIndex(const int32 InIndex) const
	{
		return Terms.IsValidIndex(InIndex);
	}
	
	NO_DISCARD FORCEINLINE int32 GetLastTermIndex() const
	{
		solid_checkf(!Terms.IsEmpty(), TEXT("No terms available to get last term index from"));
		return Terms.Num() - 1;
	}
	
	NO_DISCARD FORCEINLINE const FFlecsQueryTermExpression& GetTermAt(const int32 InIndex) const
	{
		solid_checkf(IsValidTermIndex(InIndex), TEXT("Invalid term index %d provided to GetTermAt"), InIndex);
		return Terms[InIndex];
	}
	
	NO_DISCARD FORCEINLINE FFlecsQueryTermExpression& GetTermAt(const int32 InIndex)
	{
		solid_checkf(IsValidTermIndex(InIndex), TEXT("Invalid term index %d provided to GetTermAt"), InIndex);
		return Terms[InIndex];
	}
	
	NO_DISCARD FORCEINLINE const FFlecsQueryTermExpression& GetLastTerm() const
	{
		solid_checkf(!Terms.IsEmpty(), TEXT("No terms available to get last term from"));
		return Terms.Last();
	}
	
	NO_DISCARD FORCEINLINE FFlecsQueryTermExpression& GetLastTerm()
	{
		solid_checkf(!Terms.IsEmpty(), TEXT("No terms available to get last term from"));
		return Terms.Last();
	}

	void Apply(const TSolidNotNull<const UFlecsWorld*> InWorld, flecs::query_builder<>& InQueryBuilder) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	TArray<FFlecsQueryTermExpression> Terms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	EFlecsQueryCacheType CacheType = EFlecsQueryCacheType::Default;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query", meta = (Bitmask, BitmaskEnum = "/Script/UnrealFlecs.EFlecsQueryFlags"))
	uint8 Flags = static_cast<uint8>(EFlecsQueryFlags::None);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	bool bDetectChanges = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FFlecsQueryExpression>> OtherExpressions;
	
}; // struct FFlecsQueryDefinition
