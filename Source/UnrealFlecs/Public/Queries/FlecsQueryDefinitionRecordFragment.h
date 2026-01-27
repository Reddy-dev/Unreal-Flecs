// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FlecsQueryBuilderBase.h"

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

struct FFlecsQueryDefinitionRecordFragment::FBuilder 
	: public FFlecsEntityRecord::FFragmentBuilderType<FFlecsQueryDefinitionRecordFragment>
	, public TFlecsQueryBuilderBase<FFlecsQueryDefinitionRecordFragment::FBuilder>
	
{
	using Super = FFlecsEntityRecord::FFragmentBuilderType<FFlecsQueryDefinitionRecordFragment>;
	using Super::Super;
	
	NO_DISCARD FORCEINLINE FFlecsQueryDefinition& GetQueryDefinition_Impl() const
	{
		return const_cast<FFlecsQueryDefinitionRecordFragment::FBuilder*>(this)->GetSelf().QueryDefinition;
	}
	
}; // struct FFlecsQueryDefinitionRecordFragment::FBuilder
