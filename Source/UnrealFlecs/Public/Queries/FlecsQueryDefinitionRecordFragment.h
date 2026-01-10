// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Entities/FlecsEntityRecord.h"
#include "FlecsQueryDefinition.h"

#include "FlecsQueryDefinitionRecordFragment.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsQueryDefinitionRecordFragment : public FFlecsEntityRecordFragment
{
	GENERATED_BODY()
	
public:
	FFlecsQueryDefinitionRecordFragment() = default;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flecs | Query Definition")
	FFlecsQueryDefinition QueryDefinition;
	
	virtual void PreApplyRecordToEntity(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld, const FFlecsEntityHandle& InEntityHandle) const override;
	virtual void PostApplyRecordToEntity(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld, const FFlecsEntityHandle& InEntityHandle) const override;
	
	struct FBuilder;
	
}; // struct FFlecsQueryDefinitionRecordFragment

struct FFlecsQueryDefinitionRecordFragment::FBuilder : public FFlecsEntityRecord::FFragmentBuilderType<FFlecsQueryDefinitionRecordFragment>
{
	using Super = FFlecsEntityRecord::FFragmentBuilderType<FFlecsQueryDefinitionRecordFragment>;
	using Super::Super;
	
public:
	FORCEINLINE FBuilder& Cache(const EFlecsQueryCacheType InCacheType = EFlecsQueryCacheType::Default)
	{
		this->Get().QueryDefinition.CacheType = InCacheType;
		return *this;
	}
	
	FORCEINLINE FBuilder& DetectChanges(const bool bInDetectChanges = true)
	{
		this->Get().QueryDefinition.bDetectChanges = bInDetectChanges;
		return *this;
	}
	
	FORCEINLINE FBuilder& Flags(const uint8 InFlags)
	{
		this->Get().QueryDefinition.Flags = InFlags;
		return *this;
	}
	
	FORCEINLINE FBuilder& Flags(const EFlecsQueryFlags InFlags)
	{
		this->Get().QueryDefinition.Flags = static_cast<uint8>(InFlags);
		return *this;
	}
	
	template <typename T>
	FORCEINLINE FBuilder& With()
	{
		FFlecsQueryTermExpression Term;
		Term.SetInput<T>();
		
		this->Get().QueryDefinition.AddQueryTerm(Term);
		return *this;
	}
	
	template <Unreal::Flecs::Queries::CQueryInputType TInput>
	FORCEINLINE FBuilder& With(const TInput& InInput)
	{
		FFlecsQueryTermExpression Term;
		Term.SetInput(InInput);
		
		this->Get().QueryDefinition.AddQueryTerm(Term);
		return *this;
	}
	
	template <typename T>
	FORCEINLINE FBuilder& Without()
	{
		FFlecsQueryTermExpression Term;
		Term.SetInput<T>();
		Term.bWithout = true;
		
		this->Get().QueryDefinition.AddQueryTerm(Term);
		return *this;
	}
	
	template <Unreal::Flecs::Queries::CQueryInputType TInput>
	FORCEINLINE FBuilder& Without(const TInput& InInput)
	{
		FFlecsQueryTermExpression Term;
		Term.SetInput(InInput);
		Term.bWithout = true;
		
		this->Get().QueryDefinition.AddQueryTerm(Term);
		return *this;
	}
	
	FORCEINLINE FBuilder& AddTerm(const FFlecsQueryTermExpression& InTerm)
	{
		this->Get().QueryDefinition.AddQueryTerm(InTerm);
		return *this;
	}
	
	template <Unreal::Flecs::Queries::TQueryExpressionConcept TExpression>
	FORCEINLINE FBuilder& AddExpression(const TExpression& InExpression)
	{
		this->Get().QueryDefinition.AddAdditionalExpression(InExpression);
		return *this;
	}
	
}; // struct FFlecsQueryDefinitionRecordFragment::FBuilder
