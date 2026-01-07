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
	
	
	
}; // struct FFlecsQueryDefinitionRecordFragment::FBuilder
