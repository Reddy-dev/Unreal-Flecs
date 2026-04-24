// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Types/SolidNotNull.h"

#include "FlecsEntityRecord.h"

#include "FlecsNamedEntityRecordFragment.generated.h"

// @TODO: add additional settings
USTRUCT(BlueprintType, DisplayName = "Named Entity Fragment")
struct UNREALFLECS_API FFlecsNamedEntityRecordFragment : public FFlecsEntityRecordFragment
{
	GENERATED_BODY()

public:
	FORCEINLINE FFlecsNamedEntityRecordFragment() = default;
	FORCEINLINE FFlecsNamedEntityRecordFragment(const FString& InName, const bool bInNameInheritedSubEntities = true)
		: Name(InName)
		, bNameInheritedSubEntities(bInNameInheritedSubEntities)
	{
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record Fragment")
	FString Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Entity Record Fragment")
	bool bNameInheritedSubEntities = true;
	
	virtual void PreApplyRecordToEntity(
			const TSolidNotNull<const UFlecsWorldInterfaceObject*> InFlecsWorld, const FFlecsEntityHandle& InEntityHandle) const override;
	
	struct FBuilder;
	
}; // struct FFlecsNamedEntityRecordFragment

struct FFlecsNamedEntityRecordFragment::FBuilder : public FFlecsEntityRecord::FFragmentBuilderType<FFlecsNamedEntityRecordFragment>
{
	using Super = FFlecsEntityRecord::FFragmentBuilderType<FFlecsNamedEntityRecordFragment>;
	using Super::Super;
	
public:
	FORCEINLINE FBuilder& Named(const FString& InName)
	{
		this->GetSelf().Name = InName;
		return *this;
	}
	
	FORCEINLINE FBuilder& InheritSubEntityNames(const bool bInInheritSubEntities)
	{
		this->GetSelf().bNameInheritedSubEntities = bInInheritSubEntities;
		return *this;
	}
		
}; // struct FFlecsNamedEntityRecordFragment::FBuilder