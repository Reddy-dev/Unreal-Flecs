// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Entities/FlecsEntityRecord.h"
#include "FlecsQueryDefinition.h"
#include "Enums/FlecsQueryInOut.h"
#include "Expressions/FlecsExpressionInOut.h"

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
	
	mutable int32 LastTermIndex = -1;
	
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
		LastTermIndex = this->Get().QueryDefinition.GetLastTermIndex();
		return *this;
	}
	
	template <Unreal::Flecs::Queries::CQueryInputType TInput>
	FORCEINLINE FBuilder& With(const TInput& InInput)
	{
		FFlecsQueryTermExpression Term;
		Term.SetInput(InInput);
		
		this->Get().QueryDefinition.AddQueryTerm(Term);
		LastTermIndex = this->Get().QueryDefinition.GetLastTermIndex();
		return *this;
	}
	
	template <typename T>
	FORCEINLINE FBuilder& Without()
	{
		FFlecsQueryTermExpression Term;
		Term.SetInput<T>();
		Term.bWithout = true;
		
		this->Get().QueryDefinition.AddQueryTerm(Term);
		LastTermIndex = this->Get().QueryDefinition.GetLastTermIndex();
		return *this;
	}
	
	template <Unreal::Flecs::Queries::CQueryInputType TInput>
	FORCEINLINE FBuilder& Without(const TInput& InInput)
	{
		FFlecsQueryTermExpression Term;
		Term.SetInput(InInput);
		Term.bWithout = true;
		
		this->Get().QueryDefinition.AddQueryTerm(Term);
		LastTermIndex = this->Get().QueryDefinition.GetLastTermIndex();
		return *this;
	}
	
	FORCEINLINE FBuilder& AddTerm(const FFlecsQueryTermExpression& InTerm)
	{
		this->Get().QueryDefinition.AddQueryTerm(InTerm);
		LastTermIndex = this->Get().QueryDefinition.GetLastTermIndex();
		return *this;
	}
	
	FORCEINLINE FBuilder& TermAt(const int32 InTermIndex)
	{
		solid_checkf(this->Get().QueryDefinition.IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		LastTermIndex = InTermIndex;
		return *this;
	}
	
	FORCEINLINE FBuilder& Oper(const EFlecsQueryOperator InOperator)
	{
		solid_checkf(this->Get().QueryDefinition.IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsOperQueryExpression OperExpression;
		OperExpression.Operator = InOperator;
		
		this->Get().QueryDefinition.Terms[LastTermIndex].Children.Add(TInstancedStruct<FFlecsQueryExpression>::Make(OperExpression));
		return *this;
	}
	
	FORCEINLINE FBuilder& And()
	{
		return Oper(EFlecsQueryOperator::And);
	}
	
	FORCEINLINE FBuilder& Or()
	{
		return Oper(EFlecsQueryOperator::Or);
	}
	
	FORCEINLINE FBuilder& Not()
	{
		return Oper(EFlecsQueryOperator::Not);
	}
	
	FORCEINLINE FBuilder& Optional()
	{
		return Oper(EFlecsQueryOperator::Optional);
	}
	
	FORCEINLINE FBuilder& AndFrom()
	{
		return Oper(EFlecsQueryOperator::AndFrom);
	}
	
	FORCEINLINE FBuilder& OrFrom()
	{
		return Oper(EFlecsQueryOperator::OrFrom);
	}
	
	FORCEINLINE FBuilder& InOutExpression(const EFlecsQueryInOut InInOut, const bool bStage = false)
	{
		solid_checkf(this->Get().QueryDefinition.IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsExpressionInOut InOut;
		InOut.InOut = InInOut;
		InOut.bStage = bStage;
		
		this->Get().QueryDefinition.Terms[LastTermIndex].Children.Add(TInstancedStruct<FFlecsQueryExpression>::Make(InOut));
		return *this;
	}
	
	FORCEINLINE FBuilder& In()
	{
		return InOutExpression(EFlecsQueryInOut::Read, false);
	}
	
	FORCEINLINE FBuilder& Out()
	{
		return InOutExpression(EFlecsQueryInOut::Write, false);
	}
	
	FORCEINLINE FBuilder& InOut()
	{
		return InOutExpression(EFlecsQueryInOut::ReadWrite, false);
	}
	
	FORCEINLINE FBuilder& Read()
	{
		return InOutExpression(EFlecsQueryInOut::Read, true);
	}
	
	FORCEINLINE FBuilder& Write()
	{
		return InOutExpression(EFlecsQueryInOut::Write, true);
	}
	
	FORCEINLINE FBuilder& ReadWrite()
	{
		return InOutExpression(EFlecsQueryInOut::ReadWrite, true);
	}
	
	FORCEINLINE FBuilder& Filter()
	{
		return InOutExpression(EFlecsQueryInOut::Filter, false);
	}
	
	FORCEINLINE FBuilder& ModifyLastTerm(const TFunctionRef<void(FFlecsQueryTermExpression&)>& InModifier)
	{
		solid_checkf(this->Get().QueryDefinition.IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		InModifier(this->Get().QueryDefinition.Terms[LastTermIndex]);
		return *this;
	}
	
	template <Unreal::Flecs::Queries::TQueryExpressionConcept TExpression>
	FORCEINLINE FBuilder& AddExpression(const TExpression& InExpression)
	{
		this->Get().QueryDefinition.AddAdditionalExpression(InExpression);
		return *this;
	}
	
}; // struct FFlecsQueryDefinitionRecordFragment::FBuilder
